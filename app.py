import subprocess
from flask import Flask, render_template, request, Response
import os

app = Flask(__name__)

# Compile C program if not present
if not os.path.exists('scheduler.exe' if os.name == 'nt' else 'scheduler'):
    os.system('gcc scheduler.c -o scheduler')

def run_cmd(*args):
    cmd = ['./scheduler'] + list(args)
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.stdout.strip()

@app.route('/', methods=['GET', 'POST'])
def index():
    message = ''
    events = []
    if request.method == 'POST':
        action = request.form.get('action')

        # Insert
        if action == 'insert':
            name = request.form['name']
            date = request.form['date']
            time = request.form['time']
            seats = request.form['seats']
            out = run_cmd('insert', name, date, time, seats)
            message = f'✅ Event added with ID {out}'

        # Search
        elif action == 'search':
            sid = request.form['search_id']
            out = run_cmd('search', sid)
            if out == 'NOTFOUND':
                message = '❌ Event not found'
            else:
                parts = out.split('|')
                # Convert seats to int
                events = [{
                    'id': parts[0],
                    'name': parts[1],
                    'date': parts[2],
                    'time': parts[3],
                    'seats': int(parts[4])
                }]
                message = f'🔍 Event found: ID {parts[0]}'

        # Modify
        elif action == 'modify':
            mid = request.form['modify_id']
            name = request.form.get('modify_name', '')
            date = request.form.get('modify_date', '')
            time = request.form.get('modify_time', '')
            seats = request.form.get('modify_seats', '0')
            out = run_cmd('modify', mid, name, date, time, seats)
            if out == 'OK':
                message = f'✏️ Event ID {mid} updated'
            else:
                message = f'❌ Event ID {mid} not found'

        # Delete
        elif action == 'delete':
            did = request.form['delete_id']
            out = run_cmd('delete', did)
            if out == 'OK':
                message = f'🗑️ Event ID {did} deleted'
            else:
                message = f'❌ Event ID {did} not found'

        # Generate random
        elif action == 'generate':
            count = request.form['gen_count']
            run_cmd('generate', count)
            message = f'🎲 Generated {count} random events'

    # Always fetch events for display (forward view)
    out = run_cmd('display_forward')
    if out and out != 'No events.':
        for line in out.split('\n'):
            if line:
                parts = line.split('|')
                if len(parts) == 5:
                    events.append({
                        'id': parts[0],
                        'name': parts[1],
                        'date': parts[2],
                        'time': parts[3],
                        'seats': int(parts[4])   # Convert to int here!
                    })

    # If we did a search, events only contains the search result, so we show that.
    # Otherwise events contains full list.
    return render_template('index.html', events=events, message=message)

@app.route('/reverse')
def reverse():
    out = run_cmd('display_reverse')
    events = []
    if out and out != 'No events.':
        for line in out.split('\n'):
            if line:
                parts = line.split('|')
                if len(parts) == 5:
                    events.append({
                        'id': parts[0],
                        'name': parts[1],
                        'date': parts[2],
                        'time': parts[3],
                        'seats': int(parts[4])   # Convert to int here too
                    })
    return render_template('index.html', events=events, message='🔁 Reverse order view')

@app.route('/download_csv')
def download_csv():
    out = run_cmd('display_forward')
    if not out or out == 'No events.':
        return "No data", 404

    csv_lines = ['ID,Name,Date,Time,Seats']
    for line in out.split('\n'):
        if line:
            parts = line.split('|')
            if len(parts) == 5:
                csv_lines.append(','.join(parts))
    csv_data = '\n'.join(csv_lines)

    return Response(
        csv_data,
        mimetype='text/csv',
        headers={'Content-Disposition': 'attachment; filename=events.csv'}
    )

# Optional: Clear all route – uncomment only if you added clear_all to C code
# @app.route('/clear', methods=['POST'])
# def clear_all():
#     run_cmd('clear_all')
#     return index()

if __name__ == '__main__':
    app.run(debug=True)