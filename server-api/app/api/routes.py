#--API Routes--#
from fastapi import APIRouter, WebSocket, WebSocketDisconnect, Request, Query
from app.services.notion import Notion, SortMethods
from app.core.ws_manager import ConnectionManager

manager = ConnectionManager()
router = APIRouter()
notion = Notion()

@router.get("/tasks") 
async def read_tareas():
    flag = await notion.get_tasks_api()
    if flag:
        await manager.broadcast(notion.get_task(limit=50, sort=SortMethods.EXCLUDE_DONE))
    return {"status": "ok", "flag": flag}

@router.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    client_id = await manager.connect(websocket)
    current_tasks = notion.get_task(limit=50, sort=SortMethods.EXCLUDE_DONE)

    if current_tasks:
        await websocket.send_json(current_tasks)

    try:
        while True:
            data = await websocket.receive_text()
            print(f"Mensaje recibido: {data}")
    except WebSocketDisconnect:
        manager.disconnect(client_id)
        print("Un cliente se ha desconectado.")

@router.get("/conections")
def get_conections():
    return list(manager.active_connections.keys())

@router.post("/notion-webhook")
async def notion_webhook(request: Request, type_from_query: str = Query(None, alias="type")):
    print("Notion Webhook type:", type_from_query)
    flag = await notion.get_tasks()
    if flag:
        await manager.broadcast(notion.current_tasks)
    return {"status": "ok"}