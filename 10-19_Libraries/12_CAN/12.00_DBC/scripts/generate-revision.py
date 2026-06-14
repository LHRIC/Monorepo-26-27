import os
import sys
import subprocess
import argparse
import uuid
import re


def parse_args():
    parser = argparse.ArgumentParser(
        description="Generate a new revision of a DBC file"
    )
    parser.add_argument(
        "dbc_file", help="Path to the DBC file to create a new revision for"
    )
    parser.add_argument(
        "-o", "--output", help="Output directory (defaults to same as input)"
    )
    return parser.parse_args()


def generate_and_update_uuid(file_path):
    # Generate a new UUID
    new_uuid = str(uuid.uuid4())

    try:
        # Check if the file exists
        if not os.path.exists(file_path):
            print(f"Error: File '{file_path}' not found.")
            return False

        # Read the file content
        with open(file_path, "r") as file:
            content = file.read()

        # Check if the file contains a VERSION line
        version_match = re.search(r'VERSION "([^"]*)"', content)
        if not version_match:
            print(f"Error: No VERSION line found in '{file_path}'.")
            return False

        old_uuid = version_match.group(1)

        # Find and replace the UUID in the VERSION line
        updated_content = re.sub(
            r'VERSION "([^"]*)"',
            f'VERSION "{new_uuid}"',
            content,
            count=1,  # Only replace the first occurrence
        )

        # Write the updated content back to the file
        with open(file_path, "w") as file:
            file.write(updated_content)

        print(f"UUID updated successfully in '{file_path}'")

        print(f"Old UUID: {old_uuid}")
        print(f"New UUID: {new_uuid}")
        return True

    except Exception as e:
        print(f"Error updating UUID: {e}")
        return False


def main():
    args = parse_args()

    # Validate input file
    if not os.path.isfile(args.dbc_file):
        print(f"Error: DBC file '{args.dbc_file}' not found")
        return 1

    # Set up paths
    input_path = os.path.abspath(args.dbc_file)
    input_dir = os.path.dirname(input_path)
    input_filename = os.path.basename(input_path)
    input_basename = os.path.splitext(input_filename)[0]

    output_dir = os.path.abspath(args.output) if args.output else input_dir
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Get current script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # Generate a new UUID
    generate_and_update_uuid(input_path)

    # Run dbc-to-json.py to convert the DBC file to JSON
    dbc_to_json_script = os.path.join(script_dir, "dbc-to-json.py")
    json_output_filename = f"{input_basename}.json"
    json_output_path = os.path.join(output_dir, json_output_filename)

    result = subprocess.run(
        [sys.executable, dbc_to_json_script, input_path, json_output_path],
        capture_output=True,
        text=True,
    )

    if result.returncode != 0:
        print(f"Error running dbc-to-json.py: {result.stderr}")
        return 1

    print(f"JSON file generated successfully: {json_output_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
