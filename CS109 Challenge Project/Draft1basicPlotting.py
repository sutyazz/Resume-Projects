import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.lines import Line2D

# --- Parameters ---
# Flying car (upper route) position and down-thrust properties
flying_car_pos = np.array([50, 50, 50])  # x, y, z position (upper route)
R_max = 20  # Maximum interference radius at ground level (z=0)
z_top = flying_car_pos[2]  # Upper altitude (50)

# --- Define Flight Routes ---
num_points = 100
x_route = np.linspace(0, 100, num_points)

# Upper Route (where the flying car is located)
y_route_upper = np.full(num_points, 50)
z_route_upper = np.full(num_points, 50)

# Lower Route (directly below the upper route, same x, y but at a lower altitude)
y_route_lower = np.full(num_points, 50)
z_route_lower = np.full(num_points, 30)  # e.g., 30 units altitude

# --- Down-Thrust Interference Model ---
# At a given altitude z below z_top, the interference radius scales linearly:
# radius = ((z_top - z) / z_top) * R_max
z_lower = 30  # altitude for the lower route
interference_radius = ((z_top - z_lower) / z_top) * R_max

# Compute horizontal distances (xy-plane) from flying car to each point on lower route
lower_route_xy = np.vstack((x_route, y_route_lower)).T
flying_car_xy = flying_car_pos[:2]
distances = np.linalg.norm(lower_route_xy - flying_car_xy, axis=1)

# Determine which lower route points are within the interference cone at altitude 30
interference_mask = distances < interference_radius
interference_probability = np.sum(interference_mask) / num_points
print(f"Interference probability for lower route: {interference_probability*100:.1f}%")

# --- Visualization ---
fig = plt.figure(figsize=(10, 8))
ax = fig.add_subplot(111, projection='3d')

# Plot the upper route (blue)
ax.plot(x_route, y_route_upper, z_route_upper, label="Upper Route", color="blue", linewidth=2)

# Plot the lower route (green)
ax.plot(x_route, y_route_lower, z_route_lower, label="Lower Route", color="green", linewidth=2)
# Highlight segments of the lower route within the interference zone in red
for i in range(num_points - 1):
    if interference_mask[i]:
        ax.plot(x_route[i:i+2], y_route_lower[i:i+2], z_route_lower[i:i+2],
                color="red", linewidth=3)

# Mark the flying car on the upper route
ax.scatter(flying_car_pos[0], flying_car_pos[1], flying_car_pos[2],
           color="black", s=100, label="Flying Car")

# Create the down-thrust interference cone as a surface from z=0 to z_top
theta = np.linspace(0, 2*np.pi, 30)
z_vals = np.linspace(0, z_top, 30)
Theta, Z = np.meshgrid(theta, z_vals)
# For each z, the interference radius is linearly scaled
R = ((z_top - Z) / z_top) * R_max
X = flying_car_pos[0] + R * np.cos(Theta)
Y = flying_car_pos[1] + R * np.sin(Theta)

# Plot the interference cone surface with transparency
cone_surface = ax.plot_surface(X, Y, Z, alpha=0.3, color='gray')

# Set plot limits and labels
ax.set_xlim(0, 100)
ax.set_ylim(0, 100)
ax.set_zlim(0, 60)
ax.set_xlabel("X (Distance)")
ax.set_ylabel("Y (Distance)")
ax.set_zlabel("Z (Altitude)")
ax.set_title("Probabilistic Route Planning with Down-Thrust Interference")

# Create custom legend handles since plot_surface doesn't support labels directly
custom_handles = [
    Line2D([0], [0], color="blue", lw=2, label="Upper Route"),
    Line2D([0], [0], color="green", lw=2, label="Lower Route"),
    Line2D([0], [0], color="red", lw=3, label="Interference on Lower Route"),
    Line2D([0], [0], marker="o", color="w", markerfacecolor="black", markersize=10, label="Flying Car"),
    Line2D([0], [0], marker="s", color="gray", markersize=10, alpha=0.3, linestyle='None', label="Down-Thrust Cone")
]
ax.legend(handles=custom_handles, loc="upper right")

plt.show()
