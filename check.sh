#!/bin/bash

# Define the file path
file="serialReadingsVoltage.txt"

# Initialize line counter
line_number=0

# Loop through each line in the file
while IFS= read -r line; do
    # Increment line counter
    ((line_number++))

    # Remove carriage return characters
    line=$(echo "$line" | tr -d '\r')

    # Check if the line contains "V: "
    if [[ $line == V:* ]]; then
        # Extract the V value
        v_value=$(echo "$line" | cut -d' ' -f2)
        # Compare the V value with 3.3
        if (( $(echo "$v_value < 3.3" | bc -l) )); then
            # Print the line number and the line
            echo "Line $line_number: $line"
        fi
    fi
done < "$file"
