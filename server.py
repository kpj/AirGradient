import csv
import time
from pathlib import Path

from fastapi import FastAPI, Request


app = FastAPI()
fname_data = Path("data.csv")


@app.post("/sensors/airgradient:{board_id}/measures")
async def sensor_data(board_id: str, request: Request):
    # gather data
    data = await request.json()
    data["timestamp"] = int(time.time())

    print(board_id, data)
    fields = sorted(data.keys())

    # save data
    intial_dump = not fname_data.exists()
    with fname_data.open("a") as fd:
        writer = csv.DictWriter(fd, fieldnames=fields)

        if intial_dump:
            writer.writeheader()

        writer.writerow(data)
