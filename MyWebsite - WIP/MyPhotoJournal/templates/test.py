from prettytable import PrettyTable
import pandas as pd
import os

def get_table_size(filepath):
    """
    Opens a CSV file, displays it using PrettyTable, and calculates its relative size.

    Args:
        filepath: The path to the CSV file.

    Returns:
        A tuple containing the PrettyTable object and the relative size of the table (number of rows and columns).
        Returns (None, None) if the file does not exist or if there is an error reading the file.
    """
    if not os.path.exists(filepath):
        print(f"Error: File '{filepath}' not found.")
        return None, None

    try:
        df = pd.read_csv(filepath)  # Use pandas for efficient CSV reading
        table = PrettyTable()
        table.field_names = df.columns.tolist()
        for _, row in df.iterrows():
            table.add_row(row.tolist())

        num_rows = len(table._rows)
        num_cols = len(table.field_names)
        return table, (num_rows, num_cols)
    except Exception as e:  # Handle potential errors during file reading
        print(f"Error reading CSV file: {e}")
        return None, None

# Example usage:
filepath = "Data.csv"
table, size = get_table_size(filepath)

if table:
    print("PROGRESS UPDATE: table successfully acquired from .csv file")
    if size:
        num_rows, num_cols = size
        print("PROGRESS UPDATE: number of arows and columns acquried")

code = table.get_html_string()
html_file = open('CSVIntoTable.txt', 'w')
print("PROGRESS UPDATE: html file has been re-written")
html_file = html_file.write(code)