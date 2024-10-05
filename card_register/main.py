import os
import time
import requests
from requests.auth import HTTPBasicAuth

import cv2
from PIL import Image
import pytesseract

from pn532pi import Pn532, pn532
from pn532pi import Pn532I2c


from dotenv import load_dotenv
load_dotenv()

username = os.getenv("USERNAME", "username")
password = os.getenv('PASSWORD', 'password')
endpoint = os.getenv("ENDPOINT", 'localhost:8000')

basic = HTTPBasicAuth(username, password)

uuid = ""
cid = ""


PN532_I2C = Pn532I2c(1)
nfc = Pn532(PN532_I2C)


def validate_cid(text: str):
    text = text.strip()
    return text


def read_cid():
    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)
    cap.set(cv2.CAP_PROP_EXPOSURE, 0.0001)
    ret, frame = cap.read()
    cap.release()

    if not ret:
        return None

    frame = frame[200:360, 300:]
    Image.fromarray(cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY))
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)  # Performing OTSU threshold
    ret, thresh1 = cv2.threshold(
        gray, 0, 255, cv2.THRESH_OTSU | cv2.THRESH_BINARY_INV)

    # Creating a copy of image
    im2 = thresh1.copy()

    text = pytesseract.image_to_string(im2, config='--psm 10 --oem 3')
    return text


def setup():
    nfc.begin()
    versiondata = nfc.getFirmwareVersion()

    if (not versiondata):
        print("Didn't find PN53x board")
        raise RuntimeError("Didn't find PN53x board")  # halt

    #  Got ok data, print it out!
    print(
        "Found chip PN5 {:#x} Firmware ver. {:d}.{:d}".format(
            (versiondata >> 24) & 0xFF,
            (versiondata >> 16) & 0xFF,
            (versiondata >> 8) & 0xFF)
    )

    #  configure board to read RFID tags
    nfc.SAMConfig()

    print("Waiting for an ISO14443A Card ...")


if __name__ == "__main__":
    setup()

    while True:
        time.sleep(0.1)

        success, uid = nfc.readPassiveTargetID(
            pn532.PN532_MIFARE_ISO14443A_106KBPS)

        if success:
            u = uid.hex().upper()
            print("Uid: ", u)
            c = read_cid()

            if not c:
                print(f"CID not found: {c}")
                continue

            if uuid == u and c == cid:
                print(f"{uuid} and {cid} already tried")
                continue

            cid = c
            uuid = u

            r = requests.get(
                f"http://{endpoint}/register/card/cid",
                params={"uuid": uuid,
                        "cid": cid},
                auth=basic)

            if r.status_code != 200:
                print(f"Bad response: {r.reason}")
                continue
