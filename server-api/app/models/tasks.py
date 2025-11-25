from pydantic import BaseModel
from typing import Optional

class Task(BaseModel):
    name: str
    estado: Optional[str]
    prioridad: Optional[str]

class ResumeTasks(BaseModel):
    tp: int # total_pomodoros
    cp: int # completed_pomodoros
    tt: int # total_tasks
    ct: int # completed_tasks