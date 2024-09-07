import os
import io
import base64
import requests
from niimprint import SerialTransport, PrinterClient
from requests.auth import HTTPBasicAuth
from PIL import Image


username = os.getenv("USERNAME", "username")
password = os.getenv('PASSWORD', 'password')
endpoint = os.getenv("ENDPOINT", 'localhost:8000')
basic = HTTPBasicAuth(username, password)

port = os.getenv("PORT", "/dev/ttyACM0")
uuid = ""


if __name__ == "__main__":
    transport = SerialTransport(port=port)

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
