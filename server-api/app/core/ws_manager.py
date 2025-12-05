from fastapi import WebSocket
from typing import Dict
import uuid
import json

class ConnectionManager:
    def __init__(self):
        self.active_connections: Dict[str, WebSocket] = {}

    async def connect(self, websocket: WebSocket):
        await websocket.accept()
        client_id = str(uuid.uuid4())[:8]
        self.active_connections[client_id] = websocket
        return client_id

    def disconnect(self, client_id: str):
        self.active_connections.pop(client_id, None)

    async def send_to_all(self, message: str):
        for ws in self.active_connections.values():
            await ws.send_text(message)

    async def send_to(self, client_id: str, message: str):
        websocket = self.active_connections.get(client_id)
        if websocket:
            await websocket.send_text(message)

    async def broadcast(self, message: object):
        try:
            json_string = json.dumps(message)
        except TypeError as e:
            print(f"ERROR DE SERIALIZACIÓN: {e}")
            return
        for connection in self.active_connections.values():
            await connection.send_text(json_string)