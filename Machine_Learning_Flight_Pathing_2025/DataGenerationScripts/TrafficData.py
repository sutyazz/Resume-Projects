import csv
import numpy as np

# Expanded lists for names and locations
first_names = [
    "Alice", "Bob", "Charlie", "Diana", "Evan", "Fiona", "George", "Hannah", "Ivan", "Julia",
    "Kevin", "Laura", "Michael", "Nina", "Oliver", "Paula", "Quentin", "Rachel", "Steven", "Tina",
    "Uma", "Victor", "Wendy", "Xavier", "Yvonne", "Zach", "Aaron", "Beth", "Carter", "Danielle",
    "Elijah", "Faith", "Gavin", "Hazel", "Isaac", "Jasmine", "Kyle", "Liam", "Mia", "Noah"
]

last_names = [
    "Smith", "Johnson", "Williams", "Brown", "Jones", "Miller", "Davis", "Garcia", "Rodriguez", "Wilson",
    "Anderson", "Thomas", "Taylor", "Moore", "Jackson", "Martin", "Lee", "Perez", "Thompson", "White"
]

starting_locations = [
    "Downtown Palo Alto", "Palo Alto Caltrain", "University Ave", "Foothill Expressway", "El Camino Real",
    "Page Mill Road", "Middlefield Road", "Santa Cruz Ave", "Ramona Street", "Hamilton Avenue",
    "University Park", "Alpine Drive"
]

destinations = [
    "Stanford University",
    "HP Headquarters",
    "Town & Country Mall",
    "Palo Alto Medical Center",
    "Shoreline Amphitheatre",
    "Palo Alto City Hall",
    "Stanford Shopping Center",
    "San Antonio Shopping Center",
    "Downtown Palo Alto",
    "El Camino Real",
    "Palo Alto Public Library",
    "Palo Alto Community Center",
    "Palo Alto Farmers Market",
    "Palo Alto Convention Center",
    "Rinconada Park",
    "Baylands Park"
]

# Define hours between 6 and 22 (inclusive)
hours = list(range(6, 23))

# Assign higher weights for rush hour (7-9 AM and 4-6 PM)
weights = []
for h in hours:
    if h in [7, 8, 9, 16, 17, 18]:
        weights.append(3)  # Higher weight for rush hour
    else:
        weights.append(1)
weights = np.array(weights, dtype=float)
weights = weights / weights.sum()  # Normalize to form a probability distribution

# Open a CSV file for writing the synthetic data with rush hour bias
with open("traffic_data.csv", "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    
    # Write header row
    writer.writerow([
        "User", "Time of Travel", "Starting Location", "Destination",
        "Duration (min)", "Delay (min)", "Reliability (%)"
    ])
    
    # Generate data for 1000 records
    for i in range(1000):
        # Randomly choose first and last names
        first = np.random.choice(first_names)
        last = np.random.choice(last_names)
        user = f"{first} {last}"
        
        # Generate Time of Travel using biased rush hour probability
        hour = int(np.random.choice(hours, p=weights))
        minute = np.random.randint(0, 60)
        time_str = f"{hour:02d}:{minute:02d}"
        
        # Randomly choose starting location and destination
        start = np.random.choice(starting_locations)
        dest = np.random.choice(destinations)
        
        # Compute Duration, Delay, and Reliability
        # Duration: between 10 and 40 minutes
        duration = 10 + ((i * 3) % 31)
        # Delay: between 0 and 10 minutes
        delay = (i * 2) % 11
        # Reliability decreases with delay
        reliability = 100 - (delay * 2)
        
        writer.writerow([user, time_str, start, dest, duration, delay, reliability])

print("CSV file 'traffic_data_1000_users.csv' generated with 1000 records.")
