from flask import Flask, render_template, request, redirect, url_for, flash
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go
import csv
import os
from datetime import date, timedelta
import random  # Used to select a random insult

#if username = admin
#you are sent to a page where you can add and remove users CSV files

#in progress, need to finish this so you can add integer columns via the editor, search below:
### Example: Define integer input columns
###integer_columns = ['Workout 1 hr.', 'Read 10 min.']


app = Flask(__name__)
app.secret_key = 'supersecretkey'  # Required for flash messages to work

# List of columns for the habit tracker
#COLUMNS = ['Date', 'Workout 1 hr.', 'Stretched 15 min.', 'Brushed Teeth', 'Shaved', 
#           'Read 10 min.', 'Word of the Day', 'Quote of the Day', 'Went to Church', 
#           'Something Hard', 'Cleared To Do List', 'Cleared Important Emails', 
#           'Finished HW a week out', 'Called Mom & Dad', 'Cross something off the To Do list']


###################INSULTS MODULE IS NOT CURRENTLY ENSTATED BECAUSE I ACCIDENTALLY DELETED IT####################
#need to reinstitute a call to flash insult function and need to calculate the score for her
# List of random insults
LOW_INSULTS = [
    "You're like a browser tab — open, but nobody is looking at you.",
    "You're as useful as a mute button on a mime.",
    "If common sense were a file, yours would be corrupted.",
    "You're about as sharp as a marble.",
    "You're like a TODO comment — forgotten and ignored.",
    "Your logic is as clear as a foggy day.",
    "You're the kind of person who clicks 'Reply All' by accident.",
    "You're proof that Ctrl+Z doesn't work in real life.",
    "You're like an email draft — unfinished and unsent.",
    "Your code works... until it doesn't.",
    "You have the timing of a CAPTCHA expiring.",
    "You're like a software update — no one asked for you, but here you are.",
    "If mistakes were XP, you'd be level 1000.",
    "You're proof that 'error handling' isn't just for code.",
    "You're like a wildcard import — unnecessary and confusing."
]

MEDIUM_INSULTS = [
    "You're about as useful as a comment that says 'TODO: fix this'.",
    "Your logic has more holes than Swiss cheese.",
    "You're the kind of bug that's marked 'Won't Fix' in JIRA.",
    "If problem-solving was a game, you'd still be on the tutorial.",
    "Your brain has more exceptions than try/except blocks.",
    "You must be a 'while True:' loop, because you never stop being wrong.",
    "You're the reason linters exist.",
    "You're like a race condition — nobody can predict you, and you always cause issues.",
    "You're like a deprecated function — nobody uses you anymore.",
    "Your logic is like spaghetti code — messy, tangled, and hard to follow.",
    "You'd be a great feature request... except nobody's requesting you.",
    "You're the merge conflict of social interactions.",
    "You're the missing semicolon in someone's day.",
    "You're like a '404 Page Not Found' — lost and confused.",
    "If you were any slower, you'd be a nested for-loop."
]

HIGH_INSULTS = [
    "You're like an infinite loop — useless, repetitive, and a waste of everyone's time.",
    "Your logic is so flawed, even AI couldn't predict it.",
    "You're like a memory leak — slowly draining resources and nobody knows why.",
    "You're proof that not all bugs can be fixed with a patch.",
    "Your decisions have more side effects than a bad batch of medicine.",
    "You're the reason 'Rubber Duck Debugging' was invented.",
    "You're the kind of code that makes senior devs cry.",
    "You're like a .zip file that won't unzip — frustrating and useless.",
    "You're about as useful as a scroll wheel on a touchscreen.",
    "If problem-solving was a skill, you'd still be level 1.",
    "You remind me of legacy code — old, misunderstood, and in desperate need of refactoring.",
    "You're like a pull request that's been 'Awaiting Review' for months.",
    "You have more unresolved dependencies than a broken NPM package.",
    "If your logic were a framework, it would be called 'BugJS'.",
    "You're about as useful as a single-threaded server handling 10k requests."
]

BEGRUDGING_COMPLIMENTS = [
    "You're surprisingly competent — I mean that in the best way possible.",
    "You're not as bad as you think you are — you're worse, but you're getting better!",
    "For someone who 'doesn't know what they're doing,' you're doing pretty well.",
    "You somehow always manage to pull it off, and I honestly have no idea how.",
    "You’re like a runtime optimization — you improve over time, even if nobody notices.",
    "You debug like a pro — mostly because you create so many bugs.",
    "You're actually pretty decent — which is better than most.",
    "You're like a hidden Easter egg — rare, but a pleasant surprise when found.",
    "You're smarter than you look — not that it was a high bar.",
    "You're like a 'hotfix' — unexpected, but it works.",
    "You have the patience of a senior developer dealing with junior PRs — and that’s a compliment.",
    "You're more helpful than Stack Overflow at 3 AM, which is saying a lot.",
    "You're about as rare as a 10x developer — and somehow, you might be one.",
    "You're not perfect, but you're the best kind of imperfect — functional and improving.",
    "You're like an open-source contributor — unpaid, unappreciated, but absolutely necessary."
]


# Determine level of insult
def get_random_insult(score=1):
    """Return a random insult from the appropriate list based on the level."""
    if 0 < score < 10:
        return random.choice(HIGH_INSULTS)
    elif 10 < score < 20:
        return random.choice(MEDIUM_INSULTS)
    elif 20 < score < 30:
        return random.choice(LOW_INSULTS)
    else:
        return random.choice(BEGRUDGING_COMPLIMENTS)

#Flash a random insult on habit submission
def flash_random_insult(score):
    """Flash a random insult when the app boots."""
    insult = get_random_insult(score)
    flash(insult, 'insult')


def create_csv(directory, file_name, headers=None):
    os.makedirs(directory, exist_ok=True)
    file_path = os.path.join(directory, file_name)
    if os.path.exists(file_path):
        raise FileExistsError(f"CSV file '{file_path}' already exists.")
    with open(file_path, 'w', newline='') as file:
        writer = csv.writer(file)
        if headers:
            writer.writerow(headers)
    return True

def write_to_csv(directory, file_name, rows):
    file_path = os.path.join(directory, file_name)
    with open(file_path, 'a', newline='') as file:
        writer = csv.writer(file)
        writer.writerows(rows)
    return True

def read_csv(directory, file_name):
    file_path = os.path.join(directory, file_name)
    with open(file_path, 'r') as file:
        reader = csv.reader(file)
        data = [row for row in reader]
    return data

def delete_csv(directory, file_name):
    file_path = os.path.join(directory, file_name)
    if os.path.exists(file_path):
        os.remove(file_path)
    return True

def create_new_user_csv_file(username):
    file_name = f'{username}.csv'
    directory = 'User_CSV_Files'
    file_path = os.path.join(directory, file_name)

    #Check directory for CSV files exists
    if not os.path.exists(directory):
        os.makedirs(directory)

    if os.path.exists(file_path):
        raise FileExistsError(f"CSV file '{file_path}' already exists.")
    
    #define some random initial columns
    initial_columns = ['Date', 'Workout 1 hr.', 'Stretched 15 min.', 'Brushed Teeth Morning', 'Brushed Teeth Evening', 'Read 10 min.', 'Went to Church', 'Something Hard', 'Cleared To Do List', 'Cleared Important Emails', 'Cross something off the To Do list']


    with open(file_path, 'w') as file:
        file.write(','.join(initial_columns) + '\n')
        file.write('Multipliers,' + ','.join(['1'] * (len(initial_columns) - 1)) + '\n')
        file.write(f'{str(date.today())},' + ','.join(['0'] * (len(initial_columns) - 1)) + '\n')
    
    flash(f'CSV file created for user: {username}')
    return True

def fill_missing_dates(username):
    """Ensure the CSV file has entries for all dates between the first entry date and today's date while preserving the Multipliers row as the second row."""
    directory = 'User_CSV_Files'
    file_name = f'{username}.csv'
    file_path = os.path.join(directory, file_name)

    if not os.path.exists(file_path):
        print(f"CSV file '{file_path}' does not exist.")
        return False

    # Get the list of columns and the dataframe dynamically
    df = pd.read_csv(file_path)
    COLUMNS = df.columns.tolist() 
    
    
    if 'Date' not in df.columns:
        print("No 'Date' column found in the CSV file.")
        return False

    try:
        # Separate the Multipliers row
        multipliers_row = df[df['Date'] == 'Multipliers']
        df = df[df['Date'] != 'Multipliers']  # Remove the multipliers row

        # Convert the Date column to datetime for filtering
        df['Date'] = pd.to_datetime(df['Date'], errors='coerce')
        df = df.dropna(subset=['Date'])

        first_date = df['Date'].min().date()
        today_date = date.today()

        # Generate all dates between the first date and today
        all_dates = pd.date_range(start=first_date, end=today_date).date

        # Get the existing dates in the DataFrame
        existing_dates = df['Date'].dt.date.tolist()
        
        # Identify the missing dates
        missing_dates = [d for d in all_dates if d not in existing_dates]
        
        if missing_dates:
            print(f"Missing dates found: {missing_dates}")
            
            # Create new rows for the missing dates
            new_rows = [[str(missing_date)] + [0] * (len(COLUMNS) - 1) for missing_date in missing_dates]
            
            # Format the existing 'Date' column back to string
            df['Date'] = df['Date'].dt.strftime('%Y-%m-%d')
            
            # Create a DataFrame for the new missing date rows
            missing_df = pd.DataFrame(new_rows, columns=COLUMNS)
            
            # Concatenate the existing rows with the new missing date rows
            df = pd.concat([df, missing_df], ignore_index=True)
            
            # Sort by Date
            df = df.sort_values(by='Date', ignore_index=True)

            # Reinsert the Multipliers row as the second row
            if not multipliers_row.empty:
                df = pd.concat([multipliers_row, df], ignore_index=True)

            # Write the updated DataFrame back to CSV
            df.to_csv(file_path, index=False)
            
            flash(f"Missing dates filled in CSV file for '{username}'.")
            return True
        else:
            flash(f"No missing dates found for '{username}'.")
            return True
    except Exception as e:
        flash(f"Error filling missing dates: {e}")
        return False

def get_user_csv_files():
    """Return a list of all user CSV files in the 'User_CSV_Files' directory."""
    directory = 'User_CSV_Files'
    if not os.path.exists(directory):
        return []
    return [f for f in os.listdir(directory) if f.endswith('.csv')]

def calculate_total_scores(df):
    """Calculate total score for each day by multiplying each column by its multiplier."""
    # Extract the multipliers row and drop it from the DataFrame
    multipliers_row = df[df['Date'] == 'Multipliers']
    if multipliers_row.empty:
        return df
    

    multipliers = multipliers_row.iloc[0, 1:].to_dict()  # Extract multipliers for each column
    df = df[df['Date'] != 'Multipliers']  # Remove the multipliers row
    df = df[df['Date'].notnull()]  # Remove any non-date rows

    #filter the df to just be the last 7 days
    last_7_days = date.today() - timedelta(days=7)
    df['Date'] = pd.to_datetime(df['Date'], errors='coerce')  # Ensure 'Date' is datetime
    df = df[df['Date'] >= pd.Timestamp(last_7_days)]  # Filter for the last 7 days
    

    # Convert Date to datetime for easier filtering
    df['Date'] = pd.to_datetime(df['Date'], errors='coerce')

    # Convert all other columns to numeric, multiply by multipliers, and compute total score
    for col in multipliers.keys():
        if col in df.columns:
            multiplier_value = pd.to_numeric(multipliers[col], errors='coerce')
            df[col] = pd.to_numeric(df[col], errors='coerce') * multiplier_value
    
    df['Total Score'] = df.iloc[:, 1:].sum(axis=1)  # Sum across columns, excluding the Date column

    df = df[['Date', 'Total Score']]  # Keep only Date and Total Score columns
    return df

def plot_total_scores_vs_others(current_username):
    """Create a bar graph showing total scores for each day for the current user and compare to other users."""
    directory = 'User_CSV_Files'
    user_files = get_user_csv_files()
    combined_scores = pd.DataFrame()

    for file_name in user_files:
        username = file_name.replace('.csv', '')
        file_path = os.path.join(directory, file_name)
        df = pd.read_csv(file_path)
        
        # Calculate total scores for each day
        total_scores_df = calculate_total_scores(df)
        
        # Rename the total score column to include the username
        total_scores_df.rename(columns={'Total Score': f'Total Score ({username})'}, inplace=True)
        
        if combined_scores.empty:
            combined_scores = total_scores_df
        else:
            combined_scores = pd.merge(combined_scores, total_scores_df, on='Date', how='outer')

    # Convert the 'Date' column to string to avoid issues with Plotly
    combined_scores['Date'] = pd.to_datetime(combined_scores['Date'], errors='coerce').dt.strftime('%Y-%m-%d')

    # Plot the total scores for each user
    fig = go.Figure()
    for col in combined_scores.columns:
        if col != 'Date':
            fig.add_trace(go.Bar(x=combined_scores['Date'], y=combined_scores[col], name=col))

    fig.update_layout(
        title='Total Scores Per Day (User Comparison)',
        xaxis_title='Date',
        yaxis_title='Total Score',
        barmode='group',
        template='plotly_white'
    )
    
    return fig.to_html(full_html=False)

@app.route('/add-row', methods=['POST'])
def add_row():
    """Add or update a row in the user's CSV file."""
    username = request.form['username']
    csv_path = f'User_CSV_Files/{username}.csv'
    
    # Get the list of columns dynamically
    df = pd.read_csv(csv_path)
    COLUMNS = df.columns.tolist()
    
    # Collect the row data and ensure 'Date' is the first item in the row
    row_data = {'Date': str(date.today())}  # Add Date as the first key-value

    # Define which columns require integer inputs
    integer_columns = ['Cross something off the To Do list']# Example integer input columns, this is in progress and needs to be updated to work dynamically

    # Loop through the columns and handle input types dynamically
    for col in COLUMNS[1:]:  # Skip the 'Date' column
        if col in integer_columns:
            # For integer inputs, retrieve the value and default to 0 if not provided
            row_data[col] = int(request.form.get(col, 0))
        else:
            # For checkbox inputs, set to 1 if checked, otherwise 0
            row_data[col] = 1 if request.form.get(col) == 'on' else 0
    # Load the CSV as a DataFrame
    df = pd.read_csv(csv_path)
    
    # Check if today's date already exists
    if row_data['Date'] in df['Date'].values:
        # Update existing row
        row_values = [row_data[col] for col in COLUMNS]  # Ensure the list is in COLUMNS order
        df.loc[df['Date'] == row_data['Date'], :] = row_values
    else:
        # Append a new row if date does not exist
        new_row = pd.DataFrame([row_data])
        df = pd.concat([df, new_row], ignore_index=True)
    
    # Save updated DataFrame back to CSV
    df.to_csv(csv_path, index=False)
    #print(f"Row for date {row_data['Date']} successfully updated in {csv_path}")
    return redirect(url_for('dashboard', username=username, mode_selector='Individual_Comparison'))


@app.route('/', methods=['GET', 'POST']) 
def login():
    """Login page where the user inputs their username."""
    if request.method == 'POST':
        username = request.form['username'].strip()  # Remove extra whitespace
        file_path = f'User_CSV_Files/{username}.csv'

        # If the username is 'admin', redirect to the admin page
        if username.lower() == 'admin':
            return redirect(url_for('admin'))
        
        # Check if the CSV file exists for the given username
        if not os.path.exists(file_path):
            flash('User not found. Please try again.', 'error')
            return redirect(url_for('login'))
        
        # If the file exists, fill any missing dates and redirect to the dashboard
        fill_missing_dates(username)
        return redirect(url_for('dashboard', username=username, mode_selector='Habit_Form'))
    
    return render_template('login.html')

@app.route('/admin', methods=['GET','POST'])
def admin():
    """Admin page where the admin can create a new user."""
    if request.method == 'POST':
        username = request.form['username'].strip()  # Remove extra whitespace
        csv_path = f'User_CSV_Files/{username}.csv'

        # Check if the user already exists
        if os.path.exists(csv_path):
            flash(f'User "{username}" already exists.', 'error')
        else:
            create_new_user_csv_file(username)
            flash(f'User "{username}" created successfully!', 'success')

    return render_template('admin.html')

@app.route('/delete-user-csv', methods=['POST'])
def delete_user_csv():
    """Delete the user's CSV file."""
    username = request.form['username'].strip()
    
    if username:
        directory = 'User_CSV_Files'
        file_name = f'{username}.csv'

        if delete_csv(directory, file_name):
            flash(f'CSV file for user "{username}" was successfully deleted.', 'success')
        else:
            flash(f'CSV file for user "{username}" does not exist.', 'error')

    return redirect(url_for('admin'))


@app.route('/dashboard', methods=['GET', 'POST'])
def dashboard():
    """User dashboard with plot, form, and table of data."""
    username = request.args.get('username', 'default_user')
    
    # Set edit mode from URL parameter
    #edit_mode = request.args.get('edit_mode', 'false').lower() == 'true'
    csv_path = f'User_CSV_Files/{username}.csv'

    #set Mode for dashboard display to make information more accessible 
    mode_selector = request.args.get('mode_selector', type=str)

     # Example data for predictions
    will_complete_tasks = "Yes"  # Could be "Yes" or "No"
    certainty_percentage = 85  # Percentage of certainty (e.g., 85%)
    time_needed = "2 hours"  # Estimated time to complete tasks

    #in progress, need to finish this so you can add them via the editor
    # Example: Define integer input columns
    integer_columns = ['Cross something off the To Do list']


    # If the user's CSV doesn't exist, create it
    if not os.path.exists(csv_path):
        create_new_user_csv_file(username)

    df = pd.read_csv(csv_path)
    COLUMNS = df.columns.tolist()  # Get the list of columns dynamically


    # Filter to only show the last 7 days
    last_7_days = date.today() - timedelta(days=7)
    df['Date'] = pd.to_datetime(df['Date'], errors='coerce')  # Ensure 'Date' is datetime
    df = df[df['Date'] >= pd.Timestamp(last_7_days)]  # Filter for the last 7 days

    # Get today's data
    today = str(date.today())
    if today in df['Date'].dt.strftime('%Y-%m-%d').values:
        today_row = df[df['Date'].dt.strftime('%Y-%m-%d') == today].iloc[0].to_dict()
    else:
        # If today's row doesn't exist, create a new blank entry for today
        today_row = {col: 0 for col in COLUMNS}
        today_row['Date'] = today

    # Add and remove column('Habits') logic
    if request.method == 'POST':
        # Add New Column
        if 'add_column' in request.form:
            new_column = request.form['new_column'].strip()
            multiplier = request.form['multiplier'].strip()

            if new_column:
                try:
                    multiplier_value = float(multiplier)  # Ensure it's a number
                    add_column_to_csv(username, new_column, multiplier_value)
                    print(f'Added new habit: "{new_column}" with multiplier: {multiplier_value}', 'success')
                    return redirect(url_for('dashboard', username=username, mode_selector='Habit_Form_Edit'))

                except ValueError:
                    flash('Multiplier must be a valid number.', 'error')
                
        # Remove Column
        elif 'remove_column' in request.form:
            column_to_remove = request.form['column_to_remove'].strip()
            remove_column_from_csv(username, column_to_remove)
            print(f'Removed habit: "{column_to_remove}"', 'success')
            return redirect(url_for('dashboard', username=username, mode_selector='Habit_Form_Edit'))


    # Create a dictionary of the check status for each column
    checked_values = {col: int(today_row[col]) if col in today_row else 0 for col in COLUMNS[1:]}  # 1 = checked, 0 = unchecked

    # Plot 1: Individual Habit Progress Plot
    task_columns = COLUMNS[1:] 
    df['Score'] = df[task_columns].apply(pd.to_numeric, errors='coerce').sum(axis=1)

    individual_fig = go.Figure()
    for col in task_columns:
        individual_fig.add_trace(go.Scatter(x=df['Date'], y=df[col], mode='lines+markers', name=col))

    individual_plot = individual_fig.to_html(full_html=False)

    # Plot 2: Total Score Comparison Plot
    total_scores_plot = plot_total_scores_vs_others(username)

    df = pd.read_csv(csv_path)
    user_columns = df.columns.tolist()

    return render_template(
        'dashboard.html', 
        individual_plot=individual_plot, 
        total_scores_plot=total_scores_plot, 
        username=username, 
        COLUMNS=user_columns, 
        df=df,
        checked_values=checked_values, 
        #edit_mode=edit_mode,  # Pass edit_mode to the template
        mode_selector = mode_selector, #pass the mode selector to the template
        will_complete_tasks=will_complete_tasks,
        certainty_percentage=certainty_percentage,
        time_needed=time_needed,
        integer_columns=integer_columns

    )

def remove_column_from_csv(username, column_name):
    """Removes a column from the user's CSV file."""
    file_path = f'User_CSV_Files/{username}.csv'
    if os.path.exists(file_path):
        df = pd.read_csv(file_path)
        if column_name in df.columns:
            df = df.drop(columns=[column_name])
            df.to_csv(file_path, index=False)
            print(f'Column "{column_name}" removed from {file_path}')






def add_column_to_csv(username, column_name, multiplier):
    """Adds a new column with a multiplier to the user's CSV file."""
    file_path = f'User_CSV_Files/{username}.csv'

    if os.path.exists(file_path):
        df = pd.read_csv(file_path)
        
        if column_name not in df.columns:
            # Add the new column with default data 0
            df[column_name] = 0
            df.loc[0, column_name] = multiplier  # Update the multiplier row (row 2)
            df.to_csv(file_path, index=False)
            print(f'Column "{column_name}" with multiplier "{multiplier}" added to {file_path}')

if __name__ == '__main__':
    app.run(debug=True)
