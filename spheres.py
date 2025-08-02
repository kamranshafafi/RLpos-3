import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from math import sqrt

# Sphere centers and radii
sphere_centers = [(0, 0, 0), (2, 2, 2), (-1, -1, 1), (3, 1, -1)]
sphere_radii = [3, 2, 5, 2.5]  # Changed radii to ensure joint area

# Calculate bounding box
min_coords = [min(c - r for c, r in zip(coord, sphere_radii)) for coord in zip(*sphere_centers)]
max_coords = [max(c + r for c, r in zip(coord, sphere_radii)) for coord in zip(*sphere_centers)]

# Initialize list to store coordinates in joint area
joint_area_coordinates = []

# Iterate over integer coordinates within bounding box with 0.5 precision
for x in np.arange(min_coords[0], max_coords[0] + 0.5, 0.5):
    for y in np.arange(min_coords[1], max_coords[1] + 0.5, 0.5):
        for z in np.arange(min_coords[2], max_coords[2] + 0.5, 0.5):
            # Check if coordinate lies within all spheres
            within_all_spheres = all(sqrt((x - center[0]) ** 2 + (y - center[1]) ** 2 + (z - center[2]) ** 2) <= radius for center, radius in zip(sphere_centers, sphere_radii))
            # Add coordinate to joint area if it's within all spheres
            if within_all_spheres:
                joint_area_coordinates.append((x, y, z))

# Print number of coordinates in joint area
print("Number of coordinates in joint area:", len(joint_area_coordinates))

# Plotting
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# Plot spheres and shared area
for i, (center, radius) in enumerate(zip(sphere_centers, sphere_radii), start=1):
    u = np.linspace(0, 2 * np.pi, 100)
    v = np.linspace(0, np.pi, 100)
    x = radius * np.outer(np.cos(u), np.sin(v)) + center[0]
    y = radius * np.outer(np.sin(u), np.sin(v)) + center[1]
    z = radius * np.outer(np.ones(np.size(u)), np.cos(v)) + center[2]
    ax.plot_surface(x, y, z, color=f'C{i}', alpha=0.2)
    # Plot center of each sphere
    ax.scatter(center[0], center[1], center[2], color=f'C{i}', label=f'Sphere {i} center')
    # Annotate sphere center
    ax.text(center[0], center[1], center[2], f'({center[0]}, {center[1]}, {center[2]})', color='black', fontsize=10, zorder=1)

# Plot shared area if it exists
if joint_area_coordinates:
    x_shared, y_shared, z_shared = zip(*joint_area_coordinates)
    ax.scatter(x_shared, y_shared, z_shared, color='gray', label='Shared Area')

ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')

# Create custom legend for spheres
custom_legend = [plt.Line2D([0], [0], linestyle='', marker='o', markersize=10, markerfacecolor=f'C{i}', label=f'Sphere {i}') for i in range(1, len(sphere_centers) + 1)]
ax.legend(handles=custom_legend, loc='best')

plt.show()

print("Coordinates of sphere centers:", sphere_centers)
print("Number of coordinates in joint area:", len(joint_area_coordinates))
print("Coordinates in joint area:", joint_area_coordinates)
