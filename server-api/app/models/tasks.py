from pydantic import BaseModel
from typing import Optional

class Task(BaseModel):
    name: str
    estado: Optional[str]
    prioridad: Optional[str]


