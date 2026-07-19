from pathlib import Path
from meatpy.itch50 import ITCH50MessageReader, ITCH50Writer

# 1. Define input data paths using Path objects
data_dir = Path("./data")
data_dir.mkdir(parents=True, exist_ok=True)
input_file = "/mnt/c/Users/abhis/Downloads/01302019.NASDAQ_ITCH50.gz"
output_file = data_dir / "AAPL_ONLY_ITCH50.itch50.gz"

# 2. Declare the precise symbols to extract (Strings, no manual blank-padding needed)
target_symbols = ["AAPL"]

print("Streaming and isolating AAPL shares from ITCH file...")

# 3. Stream data sequentially using MeatPy context managers
with ITCH50MessageReader(input_file) as reader:
    with ITCH50Writer(output_file, symbols=target_symbols) as writer:
        for message in reader:
            # The writer automatically tracks the dynamic Stock Directory codes 
            # and passes matching messages directly to the compressed output file
            writer.process_message(message)

print(f"Extraction complete! Filtered binary stream saved to {output_file}")
