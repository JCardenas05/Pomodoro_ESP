import os
from dotenv import load_dotenv

load_dotenv()

class Config:
    """Configuración base de la aplicación."""

    ENVIRONMENT: str = os.getenv("ENVIRONMENT", "development")
    DEBUG: bool = os.getenv("DEBUG", "false").lower() in ("1", "true", "yes")

    class Notion:
        TOKEN: str = os.getenv("NOTION_TOKEN")
        DB_ID: str = os.getenv("NOTION_DB_ID")

        @staticmethod
        def validate():
            if not Config.Notion.TOKEN or not Config.Notion.DB_ID:
                raise EnvironmentError("Faltan variables de entorno de Notion.")

Config.Notion.validate()
