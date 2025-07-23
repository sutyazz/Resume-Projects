import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Set the random seed for reproducibility
np.random.seed(42)

# Define the date range
date_range = pd.date_range(start='2022-01-01', end='2022-12-31 23:00', freq='H')

# Initialize the dataframe
weather_data = pd.DataFrame(index=date_range)

# Function to simulate temperature based on day of year
def simulate_temperature(doy):
    # Average temperature ranges for Palo Alto
    avg_temp = 60 + 15 * np.sin(2 * np.pi * (doy - 173) / 365)
    # Add daily variation
    temp = avg_temp + 5 * np.sin(2 * np.pi * weather_data.index.hour / 24)
    # Add random noise
    temp += np.random.normal(0, 2, len(weather_data))
    return temp

# Function to simulate humidity inversely related to temperature
def simulate_humidity(temp):
    humidity = 80 - (temp - 60) * 0.5
    humidity += np.random.normal(0, 5, len(weather_data))
    humidity = np.clip(humidity, 20, 100)
    return humidity

# Function to simulate wind speed
def simulate_wind_speed():
    wind_speed = np.random.normal(5, 2, len(weather_data))
    wind_speed = np.clip(wind_speed, 0, 20)
    return wind_speed

# Function to simulate wind direction
def simulate_wind_direction():
    wind_direction = np.random.uniform(0, 360, len(weather_data))
    return wind_direction

# Function to simulate precipitation
def simulate_precipitation(month):
    # Define average precipitation days per month for Palo Alto
    precip_prob = {
        1: 0.15, 2: 0.12, 3: 0.10, 4: 0.05, 5: 0.02, 6: 0.01,
        7: 0.00, 8: 0.00, 9: 0.01, 10: 0.05, 11: 0.10, 12: 0.12
    }
    precip = np.zeros(len(weather_data))
    for m in precip_prob:
        idx = weather_data.index.month == m
        precip_amount = np.random.choice([0, np.random.uniform(0.01, 0.5)], size=idx.sum(), p=[1 - precip_prob[m], precip_prob[m]])
        precip[idx] = precip_amount
    return precip

# Function to simulate visibility
def simulate_visibility(precip):
    visibility = np.where(precip > 0, np.random.uniform(1, 5, len(weather_data)), np.random.uniform(8, 10, len(weather_data)))
    return visibility

# Function to assign weather conditions
def assign_weather_condition(precip, temp):
    conditions = []
    for p, t in zip(precip, temp):
        if p > 0:
            if t <= 32:
                conditions.append('Snow')
            else:
                conditions.append('Rain')
        else:
            conditions.append('Clear')
    return conditions

# Simulate each weather parameter
day_of_year = weather_data.index.dayofyear
month = weather_data.index.month
hour = weather_data.index.hour

weather_data['Temperature (°F)'] = simulate_temperature(day_of_year)
weather_data['Relative Humidity (%)'] = simulate_humidity(weather_data['Temperature (°F)'])
weather_data['Wind Speed (mph)'] = simulate_wind_speed()
weather_data['Wind Direction (degrees)'] = simulate_wind_direction()
weather_data['Precipitation (inches)'] = simulate_precipitation(month)
weather_data['Visibility (miles)'] = simulate_visibility(weather_data['Precipitation (inches)'])
weather_data['Weather Condition'] = assign_weather_condition(weather_data['Precipitation (inches)'], weather_data['Temperature (°F)'])

# Reset index to include Date and Time as a column
weather_data.reset_index(inplace=True)
weather_data.rename(columns={'index': 'Date and Time'}, inplace=True)

# Display the first few rows of the dataset
print(weather_data.head())

# Save the dataset to a CSV file
weather_data.to_csv('palo_alto_weather_data.csv', index=False)

# Plot temperature over the year
plt.figure(figsize=(12, 6))
plt.plot(weather_data['Date and Time'], weather_data['Temperature (°F)'])
plt.title('Temperature in Palo Alto (2022)')
plt.xlabel('Date')
plt.ylabel('Temperature (°F)')
plt.show()