from enum import Enum


class StatusNum(Enum):
    pendiente = 1
    en_progreso = 2
    por_hacer = 3
    pausada = 4
    completada = 5
    na = 0

class PriorityNum(Enum):
    alta = 1
    media = 2
    baja = 3
    na = 0

class CategoryNum(Enum):
    na = 0
    estudios = 1
    trabajo = 2
    hogar = 3
    personal = 4

class Encoder:
    mapping_status = {
        'pendiente': StatusNum.pendiente,
        'en progreso': StatusNum.en_progreso,
        'por hacer': StatusNum.por_hacer,
        'pausada': StatusNum.pausada,
        'completada': StatusNum.completada,
    }

    mapping_priority = {
        'alta': PriorityNum.alta,
        'media': PriorityNum.media,
        'baja': PriorityNum.baja
    }

    mapping_category = {
        'estudios': CategoryNum.estudios,
        'trabajo': CategoryNum.trabajo,
        'hogar': CategoryNum.hogar,
        'personal': CategoryNum.personal
    }

    @staticmethod
    def status_encode(status: str) -> int:
        """Convierte un estado de texto a su valor numérico correspondiente."""
        status = status.lower().strip()
        enum_value = Encoder.mapping_status.get(status, StatusNum.na)
        return enum_value.value

    @staticmethod
    def priority_encode(priority: str) -> int:
        priority = priority.lower().strip()
        enum_value = Encoder.mapping_priority.get(priority, PriorityNum.na)
        return enum_value.value
    
    @staticmethod
    def category_encode(category: str) -> int:
        category = category.lower().strip()
        enum_value = Encoder.mapping_category.get(category, CategoryNum.na)
        return enum_value.value
