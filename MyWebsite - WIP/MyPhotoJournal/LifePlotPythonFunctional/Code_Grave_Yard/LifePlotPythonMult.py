from prettytable import PrettyTable
import pandas as pd
import os
import numpy as np
from plotly import express as px

def display_csv_plotly(filepath, x_column=None, y_column=None, plot_type='scatter'):
    """
    Displays data from a CSV file using Plotly.  Pulls multipliers from row with index 1.
    """

    try:
        df = pd.read_csv(filepath)

        if df.empty:
            print("Error: CSV file is empty or could not be read properly.")
            return

        if len(df) < 2:  # Check if there are at least two rows (header + multipliers)
            print("Error: CSV file must have at least two rows (header and multipliers).")
            return

        try:
            multipliers = df.iloc[0].to_dict() # Get multipliers from the second row (index 1)
            # Remove the 'Date' column from multipliers and convert values to numeric
            if 'Date' in multipliers:
                del multipliers['Date']  # or multipliers.pop('Date', None) for Python 3.7+

            for key in multipliers:
                multipliers[key] = pd.to_numeric(multipliers[key], errors='coerce')

            # Remove the multiplier row from the DataFrame
            df = df.drop(df.index[0])

            # Reset the index after removing the row
            df = df.reset_index(drop=True)


        except (ValueError, TypeError) as e:
            print(f"Error converting multipliers to numbers: {e}")
            return


        # Apply multipliers and convert to numeric
        for col in df.columns[1:]:  # Skip 'Date'
            if col in multipliers:
                try:
                    df[col] = pd.to_numeric(df[col], errors='coerce') * multipliers[col]
                except TypeError:
                    print(f"Warning: Could not apply multiplier to column '{col}'. Check data type.")
            else:
                df[col] = pd.to_numeric(df[col], errors='coerce')

        df['Score'] = df.iloc[:, 1:].sum(axis=1, skipna=True)

        # Create hover data string
        hover_data = {col: df[col] for col in df.columns}  # Include all columns in hover data
        hover_template = "<br>".join([f"{col}: %{{customdata[{i}]}}" for i, col in enumerate(df.columns)])
        

        if x_column and y_column and x_column in df.columns and y_column in df.columns:
            fig = px.scatter(df, x=x_column, y=y_column, 
                             title=f"Scatter Plot of {y_column} vs. {x_column}",
                             custom_data=df.columns  # Pass all columns as custom data
                             ) # Use custom hover template
    
            fig.update_traces(hovertemplate=hover_template) # Use update_traces
            fig.update_traces(mode="lines+markers") 

        elif plot_type == 'scatter_matrix':
            fig = px.scatter_matrix(df, title="Scatter Matrix Plot")  # Scatter matrix handles hover data automatically
        else:
            fig = px.scatter(df, x='Date', y='Score', 
                             title="Scatter Plot of Score per Day",
                             custom_data=df.columns, 
                             hover_template=hover_template)

        fig.write_html("my_plotly_figure.html")
        #fig.show()

    except FileNotFoundError:
        print(f"Error: File '{filepath}' not found.")
    except pd.errors.EmptyDataError:
        print("Error: CSV file is empty.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")


# Example usage (no need to pass multipliers now):
display_csv_plotly("runningdata.csv", x_column='Date', y_column='Score')