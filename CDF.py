import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D

# Define the path to your Excel file using the WSL format
file_path = '/home/kshafafi/workspace/fw-phd/results/eval.xlsx'

# Read the Excel file and specifically target 'scenarioB-3d'
df = pd.read_excel(file_path, sheet_name='free-d')

# Automatically get all column names, excluding unnamed columns
data_columns = [col for col in df.columns if not col.startswith('Unnamed')]

# Define marker styles for each CDF, ensuring enough styles for the columns
marker_styles = ['o', '^', 's', 'd', 'v', '>', '<', 'p', '*', 'h'] * ((len(data_columns) // 10) + 1)

# Define colors for each CDF
colors = plt.cm.viridis(np.linspace(0, 1, len(data_columns)))

# Plotting
plt.figure(figsize=(8, 6))

# Create custom legend handles
legend_handles = []

for idx, (col, color) in enumerate(zip(data_columns, colors)):
    # Convert to numeric, coerce errors to NaN, drop NaN, and sort the data
    sorted_data = pd.to_numeric(df[col], errors='coerce').dropna().sort_values().values
    # Calculate the CDF values
    cdf = np.linspace(0., 1., len(sorted_data))
    # Plot the CDF with a specific marker style and color for each column
    plt.plot(sorted_data, cdf, label=col, color=color, linewidth=2.0, marker=marker_styles[idx], markersize=5)
    # Add custom legend handle
    legend_handles.append(Line2D([0], [0], color=color, marker=marker_styles[idx], linestyle='-', linewidth=2.0, markersize=5, label=col))

# Adjust the horizontal axis (x-axis) limits and set custom tick marks
min_x = 0.0
max_x = 0.9
step_x = 0.1

# Adjust the vertical axis (y-axis) limits and set custom tick marks
min_y = 0
max_y = 1.0
step_y = 0.1

plt.xlim([min_x, max_x])
plt.xticks(np.arange(min_x, max_x + step_x, step=step_x))
plt.ylim([min_y, max_y])
plt.yticks(np.arange(min_y, max_y + step_y, step=step_y))

# Set labels and title with increased font weight and size
plt.xlabel('Delay (s)', fontsize=14, fontweight='bold')
plt.ylabel('CDF', fontsize=14, fontweight='bold')

# Create legend with larger font size and bold text, positioned outside
plt.legend(handles=legend_handles, fontsize=12, prop={'weight': 'bold'}, loc='upper left', bbox_to_anchor=(1, 1))
plt.grid(True, color='0.9')

# Save the plot as a PDF
output_path = '/home/kshafafi/workspace/fw-phd/results/new6.pdf'
plt.tight_layout()
plt.savefig(output_path, format='pdf', bbox_inches='tight')
plt.show()