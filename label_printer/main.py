import os
import io
import base64
import time
import requests
from niimprint import SerialTransport, PrinterClient
from requests.auth import HTTPBasicAuth
from PIL import Image

from pn532pi import Pn532, pn532
from pn532pi import Pn532I2c

username = os.getenv("USERNAME", "username")
password = os.getenv('PASSWORD', 'password')
endpoint = os.getenv("ENDPOINT", 'localhost:8000')
basic = HTTPBasicAuth(username, password)

port = os.getenv("PORT", "/dev/ttyACM0")
uuid = ""

PN532_I2C = Pn532I2c(1)
nfc = Pn532(PN532_I2C)


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
    transport = SerialTransport(port=port)
    setup()
    while True:
        time.sleep(0.1)

        success, uid = nfc.readPassiveTargetID(
            pn532.PN532_MIFARE_ISO14443A_106KBPS)

        if success:
            if uuid == uid.hex():
                continue

            uuid = uid.hex()
            r = requests.get(
                f"http://{endpoint}/project-box/assign/uuid",
                params={"uuid": uuid},
                auth=basic)

            j = r.json()
            image = Image.open(
                io.BytesIO(
                    base64.b64decode(j["image"])))

            image = image.resize((384, 192), Image.LANCZOS)
            # print(image.size)

            density = 5
            printer = PrinterClient(transport)
            printer.print_image(image, density=density)
