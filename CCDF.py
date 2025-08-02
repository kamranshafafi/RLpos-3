import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D

# Define the path to your Excel file using the WSL format
file_path = '/home/kshafafi/workspace/fw-phd/results/eval.xlsx'

# Read the Excel file and specifically target 'Sheet1'
df = pd.read_excel(file_path, sheet_name='free-t')

# Automatically get all column names, excluding unnamed columns
data_columns = [col for col in df.columns if not col.startswith('Unnamed')]

# Define marker styles for each CCDF, ensuring there are enough styles for the columns
marker_styles = ['o', '^', 's', 'd', 'v', '>', '<', 'p', '*', 'h'] * ((len(data_columns) // 10) + 1)

# Define colors for each CCDF
colors = plt.cm.viridis(np.linspace(0, 1, len(data_columns)))

# Create figure with larger font sizes
plt.figure(figsize=(8, 6))
plt.rcParams.update({'font.size': 12, 'font.weight': 'bold'})

# Create custom legend handles
legend_handles = []

for idx, (col, color) in enumerate(zip(data_columns, colors)):
    # Drop NaN values and sort the data
    sorted_data = df[col].dropna().sort_values().values
    # Calculate the CCDF values
    ccdf = 1 - np.linspace(0., 1., len(sorted_data))
    # Plot the CCDF with a specific marker style and color for each column
    plt.plot(sorted_data, ccdf, label=col, color=color, linewidth=2.0)
    # Add custom legend handle
    legend_handles.append(Line2D([0], [0], color=color, marker=marker_styles[idx], 
                               linestyle='-', linewidth=2.0, markersize=8, label=col))
    # Annotate data points with markers
    for i in range(0, len(sorted_data), 1):
        plt.scatter(sorted_data[i], ccdf[i], color=color, marker=marker_styles[idx], s=30)

# Set axis limits and ticks
min_x, max_x, step_x = 110, 600, 100
min_y, max_y, step_y = 0, 1, 0.1

plt.ylim([min_y, max_y])
plt.yticks(np.arange(min_y, max_y + step_y, step=step_y))
plt.xlim([min_x, max_x])
plt.xticks(np.arange(min_x, max_x + step_x, step=step_x))

# Set labels and title with increased font weight and size
plt.xlabel('Aggregate Throughput (Mbit/s)', fontsize=14, fontweight='bold')
plt.ylabel('CCDF', fontsize=14, fontweight='bold')

# Create legend with larger font size and bold text
plt.legend(handles=legend_handles, fontsize=12, prop={'weight': 'bold'})

# Add grid
plt.grid(True)

# Save the plot as a PDF
output_path = '/home/kshafafi/workspace/fw-phd/results/new5.pdf'
plt.savefig(output_path, format='pdf', bbox_inches='tight', dpi=300)
plt.show()