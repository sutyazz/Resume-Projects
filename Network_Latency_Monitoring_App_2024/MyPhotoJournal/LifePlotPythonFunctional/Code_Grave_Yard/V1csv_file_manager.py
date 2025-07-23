
#All Commands and parameters from this file to follow(''' is a batch comment in python for documentation strings)
'''
create_csv(directory, file_name, headers)	Creates a CSV file with headers

write_to_csv(directory, file_name, rows)	Appends rows to a CSV file

read_csv(directory, file_name)	Reads and returns CSV data as a list

delete_csv(directory, file_name)	Deletes a CSV file from the directory
'''

import csv
import os
from datetime import date

#establish standard csv columns
Standard_headers =['Date','Workout 1 hr.','Stretched 15 min.','Brushed Teeth','Shaved','Read 10 min.','Word of the Day','Quote of the Day','Went to Church','Something Hard','Cleared To Do List','Cleared Important Emails','Finished HW a week out','Called Mom & Dad','Cross something off the To Do list']
    

def create_csv(directory, file_name, headers=None):
    """
    Create a new CSV file in the specified directory with the given headers.
    Raises an error if the file already exists.
    :param directory: Directory path where the CSV file will be created.
    :param file_name: Name of the CSV file to create.
    :param headers: List of column headers (optional).
    """
    try:
        # Ensure the directory exists
        os.makedirs(directory, exist_ok=True)
        
        # Full path to the file
        file_path = os.path.join(directory, file_name)
        
        # Check if the file already exists
        if os.path.exists(file_path):
            raise FileExistsError(f"CSV file '{file_path}' already exists.")
        
        # Create the file and write the headers
        with open(file_path, 'w', newline='') as file:
            writer = csv.writer(file)
            if headers:
                writer.writerow(headers)
        
        print(f"CSV file '{file_path}' created successfully.")
        return True
    except FileExistsError as fe:
        print(f"FileExistsError: {fe}")
        return False
    except Exception as e:
        print(f"Error creating CSV file '{file_path}': {e}")
        return False

def write_to_csv(directory, file_name, rows):
    """
    Write a list of rows to an existing CSV file.
    :param directory: Directory path where the CSV file is stored.
    :param file_name: Name of the CSV file to write to.
    :param rows: List of rows to add to the CSV (list of lists).
    """
    try:
        file_path = os.path.join(directory, file_name)
        with open(file_path, 'a', newline='') as file:  # 'a' for append mode
            writer = csv.writer(file)
            writer.writerows(rows)
        
        print(f"Data written to '{file_path}' successfully.")
        return True
    except Exception as e:
        print(f"Error writing to CSV file '{file_path}': {e}")
        return False


def read_csv(directory, file_name):
    """
    Read and return the contents of a CSV file.
    :param directory: Directory path where the CSV file is stored.
    :param file_name: Name of the CSV file to read.
    :return: List of rows from the CSV file.
    """
    try:
        file_path = os.path.join(directory, file_name)
        with open(file_path, 'r') as file:
            reader = csv.reader(file)
            data = [row for row in reader]
        
        print(f"Data read from '{file_path}' successfully.")
        return data
    except FileNotFoundError:
        print(f"Error: CSV file '{file_path}' not found.")
        return []
    except Exception as e:
        print(f"Error reading CSV file '{file_path}': {e}")
        return []


def delete_csv(directory, file_name):
    """
    Delete a CSV file from the specified directory.
    :param directory: Directory path where the CSV file is stored.
    :param file_name: Name of the CSV file to delete.
    """
    try:
        file_path = os.path.join(directory, file_name)
        if os.path.exists(file_path):
            os.remove(file_path)
            print(f"CSV file '{file_path}' deleted successfully.")
            return True
        else:
            print(f"Error: CSV file '{file_path}' does not exist.")
            return False
    except Exception as e:
        print(f"Error deleting CSV file '{file_path}': {e}")
        return False

def Create_New_User_CSV_File(username):
    """
    Creates a New Users CSV File with today's starting date for the first row and all 0's
    #makes all users go into the same directory 'User_CSV_Files'
    :param username: the name of the user for file pathing

    """
    #request username in terminal
    file_name = username + '.csv'

    #set up standard variables for user entry
    directory = 'User_CSV_Files'
    file_path = os.path.join(directory, file_name)

    #establish standard csv columns
    #Standard_headers =['Date','Workout 1 hr.','Stretched 15 min.','Brushed Teeth','Shaved','Read 10 min.','Word of the Day','Quote of the Day','Went to Church','Something Hard','Cleared To Do List','Cleared Important Emails','Finished HW a week out','Called Mom & Dad','Cross something off the To Do list']
    
    #get todays date for initial row
    today = str(date.today())

    try:
        # Check if the file already exists
        if os.path.exists(file_path):
            raise FileExistsError(f"CSV file '{file_path}' already exists.")
        
        #create the New Personal CSV File in User_CSV_Files
        create_csv(directory, file_name, Standard_headers)

        #set up multiplier rows and initiate todays row
        rows = [
        ['Multipliers',1,1,1,1,1,1,1,3,6,10,4,5,1,1],
        [today, 0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        ]
        write_to_csv(directory, file_name, rows)
        return True

    except FileExistsError as fe:
        print(f"FileExistsError: {fe}")
        return False
    except Exception as e:
        print(f"Error Creating new user file '{file_path}': {e}")
        return False
    
def edit_csv_row(file_name, search_value, new_row, search_column_index=0, directory = 'User_CSV_Files'):
    """
    Edit a specific row in a CSV file.
    :param directory: Directory where the CSV file is stored.
    :param file_name: Name of the CSV file to edit.
    :param search_value: Value to search for in the specified column.
    :param new_row: New row to replace the existing one. New_row must be a comma separated string of integers.
    :param search_column_index: Column index to search in (default is 0, the first column).
    """
    try:
        file_path = os.path.join(directory, file_name)
        
        if not os.path.exists(file_path):
            raise FileNotFoundError(f"CSV file '{file_path}' does not exist.")
        
        with open(file_path, 'r') as file:
            reader = csv.reader(file)
            rows = [row for row in reader]
        
        row_found = False
        #row is a list of lists where row[i] is the row that search value returns
        for i, row in enumerate(rows):
            if len(row) > search_column_index and row[search_column_index] == search_value:
                rows[i] = [search_value] + new_row # previously was : rows[i] = new_row]  ##attempting to maintain the initial date
                row_found = True
                break
        
        if not row_found:
            raise ValueError(f"No row with value '{search_value}' found in column {search_column_index}.")
        
        #'w' mode stands for write mode
        with open(file_path, 'w', newline='') as file:
            writer = csv.writer(file)
            writer.writerows(rows)
        
        print(f"Row with '{search_value}' updated successfully in '{file_path}'.")
        return True
    except FileNotFoundError as fe:
        print(f"FileNotFoundError: {fe}")
        return False
    except ValueError as ve:
        print(f"ValueError: {ve}")
        return False
    except Exception as e:
        print(f"Error editing CSV file '{file_path}': {e}")
        return False


def Todays_Input(username) :
    """
    Update Today's Values given the Username
    :param username: the name of the user for file pathing

    """

    file_path = os.path.join('User_CSV_Files', username + '.csv')
    
    #get todays date for editing todays row
    today = str(date.today())

    # Check if the file already exists and create an empty entry for today
    if os.path.exists(file_path):
        print('Welcome Back adding today to the CSV')
        with open(file_path, 'r') as file:
            reader = csv.reader(file)
            rows = [row for row in reader]
        
        search_column_index = 0
        row_found = False
        #row is a list of lists where row[i] is the row that search value returns
        for i, row in enumerate(rows):
            if len(row) > search_column_index and row[search_column_index] == today:
                row_found = True
                break    
        if not row_found:
             write_to_csv('User_CSV_Files', username + '.csv', [[today] + [0,0,0,0,0,0,0,0,0,0,0,0,0,0]])
        
    else:
        print('This user does not exist')
        return False


    #Standard_headers =['Date','Workout 1 hr.','Stretched 15 min.','Brushed Teeth','Shaved','Read 10 min.','Word of the Day','Quote of the Day','Went to Church','Something Hard','Cleared To Do List','Cleared Important Emails','Finished HW a week out','Called Mom & Dad','Cross something off the To Do list']
    
    Todays_Entry = input("comma separate all values:")
    Todays_Entry_list = Todays_Entry.split(",")

    #accept new input only if it is the same row length as headers, otherwise ask for input again
    Num_Rows = len(Standard_headers) - 1
    while len(Todays_Entry_list) != Num_Rows:
        if len(Todays_Entry_list) > Num_Rows:
            print("Too many inputs")
        elif len(Todays_Entry_list) < Num_Rows:
            print("Too few inputs")

        Todays_Entry = input("try again...")
        Todays_Entry_list = Todays_Entry.split(",")


    #change todays row and return success
    edit_csv_row(username + '.csv', today, Todays_Entry_list)
    print("Today's input has now been updated")
    return True


if __name__ == "__main__":
    #test commands dynamically
    username = input("username:")
    UserInput = "go"
    while UserInput != "stop":
        match UserInput:
            case "new":
                Create_New_User_CSV_File(username)
            case "update":
                Todays_Input(username)
        UserInput = input("CMD:")




"""
#############################TESTING SCRIPT#####################################################
####base commands
if __name__ == "__main__":
    # Example Usage
    directory = 'User_CSV_Files'
    username = input("Enter username:")
    print("Username is: " + username)
    file_name = username + '.csv'
    headers = ['Name', 'Age', 'City']

    # Step 1: Create CSV in the 'csv_files' directory
    create_csv(directory, file_name, headers)

    # Step 2: Write Data to CSV
    rows = [
        ['Alice', 25, 'New York'],
        ['Bob', 30, 'Los Angeles'],
        ['Charlie', 22, 'Chicago']
    ]
    write_to_csv(directory, file_name, rows)


    # Step 3: Read CSV Data
    data = read_csv(directory, file_name)
    print("CSV Data:", data)

    input("Press Enter to continue and delete the current user CSV File...")
    # Step 4: Delete CSV
    delete_csv(directory, file_name)
"""



