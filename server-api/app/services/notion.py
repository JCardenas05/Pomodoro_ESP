#--Notion Service--#
import requests
from app.core.config import Config
from app.tools.encoding import Encoder, StatusNum
from app.tools.str_tools import normalize_text
from enum import Enum, auto

class Property(Enum):
    NAME = "Tarea"
    ESTADO = "Estado"
    PRIORIDAD = "Prioridad"
    ESTIMADOS = "Pomodoros Estimados"
    COMPLETADOS = "Pomodoros Completados"
    FECHA_LIM = "Fecha Límite"
    CATEGORIA = "Categoría"

class SortMethods(Enum):
    EXCLUDE_DONE = auto()
    NONE = auto()

class Notion:
    current_tasks = []

    @classmethod
    async def get_tasks_api(cls):
        url = f"https://api.notion.com/v1/databases/{Config.Notion.DB_ID}/query"
        headers = {
            "Authorization": f"Bearer {Config.Notion.TOKEN}",
            "Notion-Version": "2022-06-28",
            "Content-Type": "application/json"
        }

        response = requests.post(url, headers=headers, json={"page_size": 50})
        if response.status_code != 200:
            return {"error": response.text}

        results = response.json().get("results", [])
        tasks = []

        for item in results:
            id = item.get("id")
            props = item.get("properties", {})
            name_list = props.get(Property.NAME.value, {}).get("title", [])
            name = name_list[0]["text"]["content"] if name_list else None

            estado_raw = props.get(Property.ESTADO.value, {}).get("status", {}).get("name", "")
            prioridad_raw = props.get(Property.PRIORIDAD.value, {}).get("select", {}).get("name", "")
            estimados = props.get(Property.ESTIMADOS.value, {}).get("number", 0)
            completados = props.get(Property.COMPLETADOS.value, {}).get("number", 0)
            fecha_val = props.get(Property.FECHA_LIM.value, {}).get("date", {}).get("start", None)

            categoria_prop = props.get(Property.CATEGORIA.value, {})
            if "multi_select" in categoria_prop:
                categorias_raw = [cat.get("name") for cat in categoria_prop.get("multi_select", [])]
            elif "select" in categoria_prop:
                categorias_raw = [categoria_prop.get("select", {}).get("name")]
            else:
                categorias_raw = []

            estado = Encoder.status_encode(estado_raw)
            prioridad = Encoder.priority_encode(prioridad_raw)
            categorias = [Encoder.category_encode(cat) for cat in categorias_raw]

            tasks.append({
                "id": str(id), #id
                "tsk": normalize_text(name), #tarea
                "sts": normalize_text(estado), #estado
                "pri": normalize_text(prioridad), #prioridad
                "est": normalize_text(estimados), #estimados
                "cmp": normalize_text(completados), #completados
                "dl": normalize_text(fecha_val), #fecha limite
                "cat": normalize_text(categorias[0]) #categorias
            })
        
        print("Len stringified tasks:", len(str(tasks)))

        if tasks != cls.current_tasks:
            cls.current_tasks = tasks.copy()
            return True
        return False
    
    @classmethod
    def get_task(cls, limit: int, offset: int=0, sort: SortMethods=SortMethods.NONE):
        tasks = []
        tasks=cls.current_tasks[:limit]
        match sort:
            case SortMethods.EXCLUDE_DONE:
                print("Aplicando Exclude")
                tasks = [tk for tk in tasks if tk["sts"] != StatusNum.completada.value]
                tasks.sort(key=lambda tk: tk["pri"], reverse=False)
                
        return tasks

    
