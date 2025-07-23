import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm
from matplotlib.animation import FuncAnimation
import random
from scipy.spatial.distance import cdist
import heapq
import matplotlib.animation as animation

# Set the time step for the simulation
time_step = 1  # Time units per frame

# Define central point (reference point)
central_latitude = 37.4450  # Latitude of Palo Alto City Hall
central_longitude = -122.1500  # Longitude of Palo Alto City Hall

# Earth's radius in meters (approximate)
earth_radius = 6_371_000  # in meters

# Meters per degree latitude (approximately constant)
meters_per_degree_latitude = 111_320  # Approximate meters per degree latitude

# Function to compute meters per degree longitude at a specific latitude
def meters_per_degree_longitude(latitude):
    latitude_rad = np.deg2rad(latitude)
    return 111_320 * np.cos(latitude_rad)

# Define the starting locations with their names and approximate latitude, longitude, and altitude
starting_locations = [
    {"name": "Downtown Palo Alto", "latitude": 37.4419, "longitude": -122.1430, "altitude": 10},
    {"name": "Palo Alto Caltrain", "latitude": 37.4430, "longitude": -122.1657, "altitude": 10},
    {"name": "University Ave", "latitude": 37.4450, "longitude": -122.1630, "altitude": 10},
    {"name": "Foothill Expressway", "latitude": 37.3748, "longitude": -122.1193, "altitude": 40},
    {"name": "El Camino Real", "latitude": 37.4345, "longitude": -122.1408, "altitude": 20},
    {"name": "Page Mill Road", "latitude": 37.4228, "longitude": -122.1381, "altitude": 30},
    {"name": "Middlefield Road", "latitude": 37.4339, "longitude": -122.1297, "altitude": 10},
    {"name": "Santa Cruz Ave", "latitude": 37.4487, "longitude": -122.1833, "altitude": 15},
    {"name": "Ramona Street", "latitude": 37.4427, "longitude": -122.1613, "altitude": 10},
    {"name": "Hamilton Avenue", "latitude": 37.4457, "longitude": -122.1595, "altitude": 10},
    {"name": "University Park", "latitude": 37.4500, "longitude": -122.1700, "altitude": 15},
    {"name": "Alpine Drive", "latitude": 37.4010, "longitude": -122.2242, "altitude": 50}
]

destinations = [
    {"name": "Stanford University", "latitude": 37.4275, "longitude": -122.1697, "altitude": 30},
    {"name": "HP Headquarters", "latitude": 37.4085, "longitude": -122.1461, "altitude": 20},
    {"name": "Town & Country Mall", "latitude": 37.4389, "longitude": -122.1503, "altitude": 10},
    {"name": "Palo Alto Medical Center", "latitude": 37.4361, "longitude": -122.1709, "altitude": 15},
    {"name": "Shoreline Amphitheatre", "latitude": 37.4250, "longitude": -122.0800, "altitude": 5},
    {"name": "Palo Alto City Hall", "latitude": 37.4450, "longitude": -122.1500, "altitude": 10},
    {"name": "Stanford Shopping Center", "latitude": 37.4436, "longitude": -122.1719, "altitude":15},
    {"name": "San Antonio Shopping Center", "latitude": 37.4055, "longitude": -122.1098, "altitude":15},
    {"name": "Downtown Palo Alto", "latitude": 37.4419, "longitude": -122.1430, "altitude": 10},
    {"name": "El Camino Real", "latitude": 37.4345, "longitude": -122.1408, "altitude":20},
    {"name": "Palo Alto Public Library", "latitude": 37.4270, "longitude": -122.1440, "altitude":10},
    {"name": "Palo Alto Community Center", "latitude": 37.4499, "longitude": -122.1410, "altitude":10},
    {"name": "Palo Alto Farmers Market", "latitude": 37.4445, "longitude": -122.1605, "altitude":10},
    {"name": "Palo Alto Convention Center", "latitude": 37.4530, "longitude": -122.1820, "altitude":20},
    {"name": "Rinconada Park", "latitude": 37.4490, "longitude": -122.1430, "altitude":15},
    {"name": "Baylands Park", "latitude": 37.4558, "longitude": -122.1065, "altitude":5}
]

# Function to convert latitude and longitude to x, y coordinates
def latlon_to_xy(lat, lon):
    delta_lat = lat - central_latitude
    delta_lon = lon - central_longitude
    m_per_deg_lat = meters_per_degree_latitude
    m_per_deg_lon = meters_per_degree_longitude(central_latitude)
    x = delta_lon * m_per_deg_lon  # East-West distance in meters
    y = delta_lat * m_per_deg_lat  # North-South distance in meters
    return x, y

# Add x, y, z coordinates to each location
def add_xyz_coords(locations):
    for loc in locations:
        x, y = latlon_to_xy(loc['latitude'], loc['longitude'])
        z = loc['altitude']
        loc['x'] = x
        loc['y'] = y
        loc['z'] = z

# Add coordinates to starting locations and destinations
add_xyz_coords(starting_locations)
add_xyz_coords(destinations)

# Print the locations with their coordinates
print("Starting Locations with Coordinates:")
for loc in starting_locations:
    print(f"{loc['name']}: x={loc['x']:.2f} m, y={loc['y']:.2f} m, z={loc['z']} m")

print("\nDestinations with Coordinates:")
for loc in destinations:
    print(f"{loc['name']}: x={loc['x']:.2f} m, y={loc['y']:.2f} m, z={loc['z']} m")

# FlightPath class with modifications for time-stepped simulation and piecewise arc path
class FlightPath:
    def __init__(self, flight_id, start_time, start_loc, dest_loc):
        self.flight_id = flight_id
        self.start_time = start_time
        self.start_loc = np.array(start_loc)
        self.dest_loc = np.array(dest_loc)
        self.position = self.start_loc.copy()
        self.path = []          # Positions at each time step
        self.times = []         # Time stamps corresponding to positions
        self.metadata = {}
        self.speed = 50         # Units per time unit
        self.generate_path()

    def generate_path(self):
        # Choose a cruising altitude between 100 and 200 meters
        cruising_altitude = random.uniform(100, 200)
        
        # Define three segments: vertical ascent, horizontal cruise, vertical descent
        # Segment 1: Vertical ascent from start to cruising altitude
        P0 = self.start_loc
        P1 = np.array([P0[0], P0[1], cruising_altitude])
        # Segment 2: Horizontal cruise at cruising altitude from start x,y to destination x,y
        P2 = np.array([self.dest_loc[0], self.dest_loc[1], cruising_altitude])
        # Segment 3: Vertical descent from cruising altitude to destination
        P3 = self.dest_loc
        
        # For simplicity, use fixed step counts for each segment
        seg1_steps = 10
        seg2_steps = 30
        seg3_steps = 10
        total_steps = seg1_steps + seg2_steps + seg3_steps
        
        # Generate linear interpolation for each segment
        seg1 = np.linspace(P0, P1, seg1_steps, endpoint=False)
        seg2 = np.linspace(P1, P2, seg2_steps, endpoint=False)
        seg3 = np.linspace(P2, P3, seg3_steps, endpoint=True)
        
        # Concatenate segments to form full flight path
        self.path = np.concatenate((seg1, seg2, seg3), axis=0)
        
        # Compute distances for each segment
        seg1_distance = abs(P1[2] - P0[2])
        seg2_distance = np.linalg.norm(P2[:2] - P1[:2])
        seg3_distance = abs(P3[2] - P2[2])
        total_distance = seg1_distance + seg2_distance + seg3_distance
        
        # Estimate travel time based on total distance and speed
        travel_time = total_distance / self.speed
        self.times = np.linspace(self.start_time, self.start_time + travel_time, total_steps)
        
        # Store metadata
        self.metadata['planned_path'] = self.path
        self.metadata['expected_travel_time'] = travel_time
        self.metadata['cruising_altitude'] = cruising_altitude

# Function to adjust flight paths to avoid down-thrust zones
def adjust_paths(flight_paths):
    for i, fp in enumerate(flight_paths):
        adjusted = False
        attempts = 0
        max_attempts = 10
        while not adjusted and attempts < max_attempts:
            other_zones = create_down_thrust_zones(flight_paths[:i])
            if path_intersects_zones(fp.path, other_zones):
                fp.start_loc[2] += 5
                fp.dest_loc[2] += 5
                fp.generate_path()
                attempts += 1
            else:
                adjusted = True
        if not adjusted:
            print(f"Warning: Could not adjust path for Flight {fp.flight_id} to avoid down-thrust zones.")

# Function to generate down-thrust interference zones
def create_down_thrust_zones(flight_paths):
    zones = []
    for fp in flight_paths:
        zone = {'center': fp.position, 'radius': 5, 'height': 20}
        zones.append(zone)
    return zones

# Function to check if a path intersects any down-thrust zones
def path_intersects_zones(path, zones):
    for point in path:
        for zone in zones:
            dx = point[0] - zone['center'][0]
            dy = point[1] - zone['center'][1]
            dz = point[2] - zone['center'][2]
            if dz < 0 and abs(dz) <= zone['height']:
                distance_xy = np.sqrt(dx**2 + dy**2)
                if distance_xy <= zone['radius']:
                    return True
    return False

# Function to simulate weather volatility affecting the flight paths
def apply_weather_effects(flight_paths):
    for fp in flight_paths:
        volatility = np.random.normal(0, 1, fp.path.shape)
        weather_influence = 0.1
        fp.path += weather_influence * volatility
        fp.metadata['weather_volatility'] = volatility
        fp.metadata['weather_influenced_path'] = fp.path.copy()

# Function to simulate dynamic updates to the flight paths
def dynamic_updates(flight_paths):
    update_time = 50
    for fp in flight_paths:
        if fp.start_time < update_time < fp.start_time + fp.metadata['expected_travel_time']:
            fp.generate_path()
            adjust_paths([fp])
            print(f"Dynamic update applied to Flight {fp.flight_id}")

# Main function to run the simulation
def simulate_flights(starting_locations, destinations, num_flights):
    flight_paths = []
    for i in range(num_flights):
        start_loc = starting_locations[i % len(starting_locations)]
        dest_loc = destinations[i % len(destinations)]
        start_time = random.uniform(0, 5)
        fp = FlightPath(
            flight_id=i+1,
            start_time=start_time,
            start_loc=np.array([start_loc['x'], start_loc['y'], start_loc['z']]),
            dest_loc=np.array([dest_loc['x'], dest_loc['y'], dest_loc['z']])
        )
        flight_paths.append(fp)
    
    last_arrival_time = max(fp.times[-1] for fp in flight_paths)
    total_simulation_time = last_arrival_time + 10

    adjust_paths(flight_paths)
    apply_weather_effects(flight_paths)
    dynamic_updates(flight_paths)
    visualize_flight_paths(flight_paths, total_simulation_time)
    return flight_paths

# Function to visualize the flight paths with animation
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
        line, = ax.plot([], [], [], label=f'Flight {fp.flight_id}', linewidth=2)
        lines.append(line)
        point, = ax.plot([], [], [], 'o', markersize=8)
        points.append(point)
        ax.scatter(fp.start_loc[0], fp.start_loc[1], fp.start_loc[2], marker='o', color='green', s=50)
        ax.scatter(fp.dest_loc[0], fp.dest_loc[1], fp.dest_loc[2], marker='x', color='red', s=50)

    ax.set_xlabel('X Coordinate (m)')
    ax.set_ylabel('Y Coordinate (m)')
    ax.set_zlabel('Altitude (m)')
    ax.legend()
    plt.title('Flight Paths of eVTOLs with Vertical Takeoff and Descent')

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

# Run the simulation with 5 flight paths
if __name__ == '__main__':
    num_flights = 5  # Can be scaled to more flights
    simulate_flights(starting_locations, destinations, num_flights)
