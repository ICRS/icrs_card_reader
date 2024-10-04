# ICRS Boxes - Label Printer

Print labels for personal storage boxes.

# Dependencies:
Need to install python3 dev:
RPi/Ubuntu
`sudo apt-get install python3-dev`

# Setup
1. Clone the code
2. Create and activate a virtual environment: `python -m venv .venv`
3. Install dependencies `pip install -r requirements.txt`
4. Pull submodules `git submodule update --init --recursive`
5. Install Niimbot dependency: `cd niimprint; poetry install` 


# Run
Run `python main.py`
