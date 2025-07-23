import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import matplotlib.animation as animation
import pandas as pd
import random
from scipy.spatial.distance import cdist
import heapq
import datetime
from scipy.stats import norm

# -----------------------------
# Global Time Step for Simulation
# -----------------------------
time_step = 1  # Time units per frame

# -----------------------------
# Data Collection: Load Weather and Traffic Data
# -----------------------------
try:
    weather_df = pd.read_csv("palo_alto_weather_data.csv")
except Exception as e:
    print("Warning: Could not read palo_alto_weather_data.csv. Using simulated weather data.")
    weather_df = pd.DataFrame({
        'timestamp': pd.date_range("2023-01-01", periods=100, freq='H'),
        'wind_speed': np.random.uniform(0, 10, 100)
    })

try:
    traffic_df = pd.read_csv("traffic_data.csv")
except Exception as e:
    print("Warning: Could not read traffic_data.csv. Using simulated traffic data.")
    traffic_df = pd.DataFrame({
        'User': ['Evan Anderson', 'Oliver Smith', 'Nina Martin'],
        'Time of Travel': ["18:59", "06:47", "17:46"],
        'Starting Location': ["University Park", "University Ave", "Downtown Palo Alto"],
        'Destination': ["Stanford Shopping Center", "Palo Alto City Hall", "Palo Alto Public Library"],
        'Duration (min)': [10, 13, 16],
        'Delay (min)': [0, 2, 4],
        'Reliability (%)': [100, 96, 92]
    })

# -----------------------------
# Helper functions for Weather and Traffic Risk
# -----------------------------
def get_weather_risk(planned_travel_time):
    """
    Bootstraps from historical weather data to estimate the probability of adverse weather.
    Assumes wind speeds > 7 m/s indicate high risk.
    """
    for col_name in ['wind_speed', 'Wind Speed', 'windspeed', 'WindSpeed']:
        if col_name in weather_df.columns:
            samples = weather_df[col_name].values
            break
    else:
        print("Warning: No wind speed column found in weather data. Using default simulated values.")
        samples = np.random.uniform(0, 10, 100)
    
    threshold = 7.0  # m/s threshold for adverse conditions
    num_samples = 1000
    bootstrap_samples = np.random.choice(samples, size=num_samples, replace=True)
    risk_probability = np.mean(bootstrap_samples > threshold)
    confidence = np.std(bootstrap_samples > threshold)
    return risk_probability, confidence

def get_traffic_risk(planned_start_time, region="Palo Alto"):
    """
    Estimates traffic risk based on the number of flights scheduled near the planned start time.
    """
    def time_str_to_hour(time_str):
        parts = time_str.split(':')
        return int(parts[0]) + int(parts[1]) / 60.0
    planned_hour = planned_start_time
    traffic_hours = traffic_df['Time of Travel'].apply(time_str_to_hour)
    count = traffic_hours[(traffic_hours >= planned_hour - 1) & (traffic_hours <= planned_hour + 1)].count()
    risk = min(count / 10.0, 1.0)
    return risk

# -----------------------------
# Predefined Locations and Coordinate Conversion
# -----------------------------
central_latitude = 37.4450
central_longitude = -122.1500
meters_per_degree_latitude = 111320
def meters_per_degree_longitude(latitude):
    return 111320 * np.cos(np.deg2rad(latitude))

def latlon_to_xy(lat, lon):
    delta_lat = lat - central_latitude
    delta_lon = lon - central_longitude
    x = delta_lon * meters_per_degree_longitude(central_latitude)
    y = delta_lat * meters_per_degree_latitude
    return x, y

# Predefined starting locations – extended to include "University Park"
starting_locations = [
    {"name": "Downtown Palo Alto", "latitude": 37.4419, "longitude": -122.1430, "altitude": 10},
    {"name": "Palo Alto Caltrain", "latitude": 37.4430, "longitude": -122.1657, "altitude": 10},
    {"name": "University Ave", "latitude": 37.4450, "longitude": -122.1630, "altitude": 10},
    {"name": "Foothill Expressway", "latitude": 37.3748, "longitude": -122.1193, "altitude": 40},
    {"name": "El Camino Real", "latitude": 37.4345, "longitude": -122.1408, "altitude": 20},
    {"name": "Page Mill Road", "latitude": 37.4228, "longitude": -122.1381, "altitude": 30},
    {"name": "Middlefield Road", "latitude": 37.4339, "longitude": -122.1297, "altitude": 10},
    {"name": "University Park", "latitude": 37.4500, "longitude": -122.1700, "altitude": 15}
]

# Predefined destinations – extended to include "Stanford Shopping Center" and "Palo Alto Public Library"
destinations = [
    {"name": "Stanford University", "latitude": 37.4275, "longitude": -122.1697, "altitude": 30},
    {"name": "HP Headquarters", "latitude": 37.4085, "longitude": -122.1461, "altitude": 20},
    {"name": "Town & Country Mall", "latitude": 37.4389, "longitude": -122.1503, "altitude": 10},
    {"name": "Palo Alto Medical Center", "latitude": 37.4361, "longitude": -122.1709, "altitude": 15},
    {"name": "Palo Alto City Hall", "latitude": 37.4450, "longitude": -122.1500, "altitude": 10},
    {"name": "Stanford Shopping Center", "latitude": 37.4436, "longitude": -122.1719, "altitude": 15},
    {"name": "Palo Alto Public Library", "latitude": 37.4270, "longitude": -122.1440, "altitude": 10}
]

def add_xyz_coords(locations):
    for loc in locations:
        x, y = latlon_to_xy(loc['latitude'], loc['longitude'])
        loc['x'] = x
        loc['y'] = y
        loc['z'] = loc['altitude']

add_xyz_coords(starting_locations)
add_xyz_coords(destinations)

# -----------------------------
# FlightPath Class and Route Optimization
# -----------------------------
class FlightPath:
    def __init__(self, flight_id, start_time, start_loc, dest_loc, cruising_altitude):
        self.flight_id = flight_id
        self.start_time = start_time
        self.start_loc = np.array(start_loc)
        self.dest_loc = np.array(dest_loc)
        self.cruising_altitude = cruising_altitude
        self.path = None
        self.times = None
        self.metadata = {}
        self.speed = 50  # units per time unit
        self.generate_path()

    def generate_path(self):
        # Build a piecewise path: vertical ascent, horizontal cruise, vertical descent.
        P0 = self.start_loc
        P1 = np.array([P0[0], P0[1], self.cruising_altitude])
        P2 = np.array([self.dest_loc[0], self.dest_loc[1], self.cruising_altitude])
        P3 = self.dest_loc

        seg1_steps = 10
        seg2_steps = 30
        seg3_steps = 10
        total_steps = seg1_steps + seg2_steps + seg3_steps

        seg1 = np.linspace(P0, P1, seg1_steps, endpoint=False)
        seg2 = np.linspace(P1, P2, seg2_steps, endpoint=False)
        seg3 = np.linspace(P2, P3, seg3_steps, endpoint=True)
        self.path = np.concatenate((seg1, seg2, seg3), axis=0)

        seg1_distance = abs(P1[2] - P0[2])
        seg2_distance = np.linalg.norm(P2[:2] - P1[:2])
        seg3_distance = abs(P3[2] - P2[2])
        total_distance = seg1_distance + seg2_distance + seg3_distance
        travel_time = total_distance / self.speed
        self.times = np.linspace(self.start_time, self.start_time + travel_time, total_steps)
        self.metadata['planned_path'] = self.path
        self.metadata['expected_travel_time'] = travel_time

def optimize_flight_path(flight_id, start_time, start_loc, dest_loc):
    best_cost = float('inf')
    best_fp = None
    candidate_altitudes = np.arange(100, 210, 10)
    for alt in candidate_altitudes:
        fp = FlightPath(flight_id, start_time, start_loc, dest_loc, cruising_altitude=alt)
        travel_time = fp.metadata['expected_travel_time']
        weather_risk, weather_conf = get_weather_risk(travel_time)
        traffic_risk = get_traffic_risk(start_time)
        combined_risk = 0.5 * weather_risk + 0.5 * traffic_risk
        cost = travel_time + 100 * combined_risk
        if cost < best_cost:
            best_cost = cost
            best_fp = fp
            best_fp.metadata['combined_risk'] = combined_risk
    return best_fp

# -----------------------------
# Collision Avoidance: Adjust paths to avoid collisions
# -----------------------------
def avoid_collisions(flight_paths):
    safety_altitude_diff = 20  # Minimum vertical separation required (m)
    for i in range(len(flight_paths)):
        for j in range(i+1, len(flight_paths)):
            fp_i = flight_paths[i]
            fp_j = flight_paths[j]
            # Check if active time intervals overlap
            if (fp_i.start_time <= fp_j.start_time <= fp_i.times[-1]) or (fp_j.start_time <= fp_i.start_time <= fp_j.times[-1]):
                if abs(fp_i.cruising_altitude - fp_j.cruising_altitude) < safety_altitude_diff:
                    fp_j.cruising_altitude += safety_altitude_diff
                    fp_j.generate_path()
                    print(f"Adjusted Flight {fp_j.flight_id}'s cruising altitude to {fp_j.cruising_altitude} to avoid collision with Flight {fp_i.flight_id}")

# -----------------------------
# Functions to Simulate Dynamic Updates (Placeholders)
# -----------------------------
def adjust_paths(flight_paths):
    pass

def dynamic_updates(flight_paths):
    update_time = 50
    for fp in flight_paths:
        if fp.start_time < update_time < fp.start_time + fp.metadata['expected_travel_time']:
            fp.generate_path()
            print(f"Dynamic update applied to Flight {fp.flight_id}")

# -----------------------------
# New Function: Parse Time String to Hour Float
# -----------------------------
def time_str_to_float(time_str):
    parts = time_str.split(':')
    return int(parts[0]) + int(parts[1]) / 60.0

# -----------------------------
# New Function: Get Active Traffic Flights at User Start Time
# -----------------------------
def get_active_traffic_flights(user_start_time):
    active_flights = []
    for idx, row in traffic_df.iterrows():
        flight_start = time_str_to_float(row["Time of Travel"])
        duration_hours = row["Duration (min)"] / 60.0
        flight_end = flight_start + duration_hours
        if flight_start <= user_start_time <= flight_end:
            start_name = row["Starting Location"]
            dest_name = row["Destination"]
            start_loc = None
            for loc in starting_locations:
                if loc["name"] == start_name:
                    start_loc = [loc["x"], loc["y"], loc["z"]]
                    break
            if start_loc is None:
                start_loc = [0, 0, 10]
            dest_loc = None
            for loc in destinations:
                if loc["name"] == dest_name:
                    dest_loc = [loc["x"], loc["y"], loc["z"]]
                    break
            if dest_loc is None:
                dest_loc = [0, 0, 10]
            fp = FlightPath(flight_id=f"Traffic_{row['User']}", 
                            start_time=flight_start,
                            start_loc=start_loc,
                            dest_loc=dest_loc,
                            cruising_altitude=150)
            active_flights.append(fp)
    return active_flights

# -----------------------------
# New Function: Compute Downwash Interference Probability
# -----------------------------
def compute_downwash_probability(user_fp, other_flights):
    user_mid = user_fp.path[len(user_fp.path)//2]
    prob_sum = 0
    count = 0
    for fp in other_flights:
        if fp.flight_id != user_fp.flight_id:
            if fp.cruising_altitude > user_fp.cruising_altitude:
                fp_mid = fp.path[len(fp.path)//2]
                horizontal_distance = np.linalg.norm(user_mid[:2] - fp_mid[:2])
                vertical_diff = fp.cruising_altitude - user_fp.cruising_altitude
                if horizontal_distance < 500 and vertical_diff < 50:
                    p = 0.5
                elif horizontal_distance < 1000:
                    p = 0.2
                else:
                    p = 0.05
                prob_sum += p
                count += 1
    if count > 0:
        return min(prob_sum / count, 1.0)
    else:
        return 0.0

# -----------------------------
# New Function: Compute Arrival Probability by a Target Time
# -----------------------------
def compute_arrival_probability(fp, target_time):
    mean_time = fp.metadata['expected_travel_time']
    sigma = 0.1 * mean_time + 5 * fp.metadata.get('combined_risk', 0)
    arrival_prob = norm.cdf(target_time, loc=mean_time, scale=sigma)
    return arrival_prob

# -----------------------------
# Visualization and Animation
# -----------------------------
def visualize_flight_paths(flight_paths, total_simulation_time):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    
    all_x = np.concatenate([fp.path[:, 0] for fp in flight_paths])
    all_y = np.concatenate([fp.path[:, 1] for fp in flight_paths])
    all_z = np.concatenate([fp.path[:, 2] for fp in flight_paths])
    ax.set_xlim([np.min(all_x) - 1000, np.max(all_x) + 1000])
    ax.set_ylim([np.min(all_y) - 1000, np.max(all_y) + 1000])
    ax.set_zlim([0, np.max(all_z) + 50])
    
    lines = []
    points = []
    for fp in flight_paths:
        if isinstance(fp.flight_id, int):
            line_style = '-'
            marker_color = 'blue'
            label = f'Flight {fp.flight_id} (You)'
        else:
            line_style = '--'
            marker_color = 'gray'
            label = f'{fp.flight_id}'
        line, = ax.plot([], [], [], line_style, label=label, linewidth=2)
        lines.append(line)
        point, = ax.plot([], [], [], 'o', markersize=8, color=marker_color)
        points.append(point)
        ax.scatter(fp.start_loc[0], fp.start_loc[1], fp.start_loc[2], marker='o', color='green', s=50)
        ax.scatter(fp.dest_loc[0], fp.dest_loc[1], fp.dest_loc[2], marker='x', color='red', s=50)
    
    ax.set_xlabel('X Coordinate (m)')
    ax.set_ylabel('Y Coordinate (m)')
    ax.set_zlabel('Altitude (m)')
    ax.legend()
    plt.title('Optimized Flight Path with Current Traffic and Collision Avoidance')
    
    def update(frame):
        current_time = frame * time_step
        for i, fp in enumerate(flight_paths):
            if fp.start_time <= current_time <= fp.times[-1]:
                idx = np.searchsorted(fp.times, current_time)
                idx = min(idx, len(fp.path) - 1)
                lines[i].set_data(fp.path[:idx+1, 0], fp.path[:idx+1, 1])
                lines[i].set_3d_properties(fp.path[:idx+1, 2])
                points[i].set_data([fp.path[idx, 0]], [fp.path[idx, 1]])
                points[i].set_3d_properties([fp.path[idx, 2]])
            else:
                lines[i].set_data([], [])
                lines[i].set_3d_properties([])
                points[i].set_data([], [])
                points[i].set_3d_properties([])
        return lines + points
    
    num_frames = int(total_simulation_time / time_step)
    ani = animation.FuncAnimation(fig, update, frames=num_frames, interval=100, blit=False)
    plt.show()

# -----------------------------
# User Interface for Input (Console-based)
# -----------------------------
def user_input():
    print("Available Starting Locations:")
    for i, loc in enumerate(starting_locations):
        print(f"{i+1}: {loc['name']}")
    start_choice = int(input("Choose a starting location (number): ")) - 1

    print("\nAvailable Destinations:")
    for i, loc in enumerate(destinations):
        print(f"{i+1}: {loc['name']}")
    dest_choice = int(input("Choose a destination (number): ")) - 1

    start_time = float(input("\nEnter desired start time (in hours, e.g., 8.5 for 8:30 AM): "))
    chosen_start = starting_locations[start_choice]
    chosen_dest = destinations[dest_choice]
    return chosen_start, chosen_dest, start_time

# -----------------------------
# Main Simulation Function
# -----------------------------
def simulate():
    chosen_start, chosen_dest, user_start_time = user_input()
    optimized_fp = optimize_flight_path(
        flight_id=1,
        start_time=user_start_time,
        start_loc=np.array([chosen_start['x'], chosen_start['y'], chosen_start['z']]),
        dest_loc=np.array([chosen_dest['x'], chosen_dest['y'], chosen_dest['z']])
    )
    print(f"\nOptimized Flight Path for your trip:")
    print(f"  Expected Travel Time: {optimized_fp.metadata['expected_travel_time']:.2f} time units")
    print(f"  Weather Risk: {get_weather_risk(optimized_fp.metadata['expected_travel_time'])[0]:.2f}")
    print(f"  Traffic Risk: {get_traffic_risk(user_start_time):.2f}")
    print(f"  Combined Risk: {optimized_fp.metadata.get('combined_risk', 0):.2f}")
    
    # Ask the user for a target arrival time (in time units)
    target_time = float(input("Enter a target arrival time (in time units, relative to your start time): "))
    arrival_prob = compute_arrival_probability(optimized_fp, target_time)
    print(f"  Probability of arriving by target time: {arrival_prob:.2f}")
    
    # Get active traffic flights at the user's start time.
    traffic_flights = get_active_traffic_flights(user_start_time)
    print(f"\nFound {len(traffic_flights)} active traffic flights at your start time.")
    
    # Compute downwash interference probability
    downwash_prob = compute_downwash_probability(optimized_fp, traffic_flights)
    print(f"  Estimated Downwash Interference Probability: {downwash_prob:.2f}")
    
    all_flights = [optimized_fp] + traffic_flights
    dynamic_updates(all_flights)
    adjust_paths(all_flights)
    avoid_collisions(all_flights)
    total_simulation_time = max(fp.times[-1] for fp in all_flights) + 10
    visualize_flight_paths(all_flights, total_simulation_time)

# Run the simulation
if __name__ == '__main__':
    simulate()
