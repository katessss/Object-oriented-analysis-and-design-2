import os
from fastapi import FastAPI
from fastapi.responses import HTMLResponse
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from abc import ABC, abstractmethod
from pathlib import Path
from dotenv import load_dotenv
from logger_config import setup_logger
logger = setup_logger(__name__)
from models import giga, openAI, deepseek

load_dotenv()

NOT_KNOW = "НЕ ЗНАЮ"

app = FastAPI(title="Support Chain API")
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)


class Ticket:
    def __init__(self, content: str, level: int):
        self.content = content
        self.level = level
        self.history: list[str] = []    
        self.llm_logs: dict[str, str] = {}

    def add_log(self, name: str):
        self.history.append(name)


class SupportHandler(ABC):
    def __init__(self, next_handler=None):
        self._next = next_handler

    def set_next(self, h):
        self._next = h
        return h

    @abstractmethod
    def handle(self, ticket: Ticket) -> str:
        if self._next:
            return self._next.handle(ticket)
        return "Тикет закрыт: ни один специалист не смог помочь."


class BotHandler(SupportHandler):
    def handle(self, ticket: Ticket) -> str:
        if "пароль" in ticket.content.lower():
            return "Бот: Я сбросил ваш пароль!"
        if "оплата" in ticket.content.lower():
            return "Оператор: Я проверил вашу оплату, всё ок."
        
        ticket.add_log("Бот-автоответчик (GigaChat)")
        answer = giga(ticket.content)
        ticket.llm_logs["bot"] = answer
        if NOT_KNOW in answer.upper():
            logger.info("Бот не может ответить на этот вопрос")
            return super().handle(ticket)
        return answer


class OperatorHandler(SupportHandler):
    def handle(self, ticket: Ticket) -> str:
        logger.info("Оператор 2-й линии (DeepSeek) думает...")
        ticket.add_log("Оператор 2-й линии (DeepSeek)")
        answer = deepseek(ticket.content)
        ticket.llm_logs["op"] = answer
        if NOT_KNOW in answer.upper():
            return super().handle(ticket)
        return answer


class LeadHandler(SupportHandler):
    def handle(self, ticket: Ticket) -> str:
        logger.info("Техлид думает...")
        ticket.add_log("Техлид (OpenAI)")
        answer = openAI(ticket.content)
        ticket.llm_logs["lead"] = answer
        if NOT_KNOW in answer.upper():
            return super().handle(ticket)
        return answer


class SupportDepartment:
    def __init__(self):
        self.head = None  # Начало цепочки
        self.tail = None  # Конец цепочки

    def add_specialist(self, handler: SupportHandler):
        if not self.head:
            self.head = handler
            self.tail = handler
        else:
            self.tail.set_next(handler)
            self.tail = handler
        return self # Для цепочечных вызовов
    

def who_resolved(path: list[str], result: str) -> str:
    if not path or "не смог" in result:
        return "none"
    last = path[-1]
    if "GigaChat" in last: return "bot"
    if "DeepSeek" in last: return "op"
    if "OpenAI"   in last: return "lead"
    return "none"


class TicketRequest(BaseModel):
    content: str
    level: int = 1


class TicketResponse(BaseModel):
    result: str
    path: list[str]
    resolved_by: str
    llm_logs: dict[str, str] = {}


@app.get("/", response_class=HTMLResponse)
async def root():
    return HTMLResponse(Path("index.html").read_text(encoding="utf-8"))


@app.post("/handle", response_model=TicketResponse)
async def handle_ticket(req: TicketRequest):
    ticket = Ticket(content=req.content, level=req.level)
    department = SupportDepartment()
    department.add_specialist(BotHandler())\
        .add_specialist(OperatorHandler())\
            .add_specialist(LeadHandler())
    result = department.head.handle(ticket)

    return TicketResponse(
        result=result,
        path=ticket.history,
        resolved_by=who_resolved(ticket.history, result),
        llm_logs=ticket.llm_logs,
    )


@app.get("/health")
async def health():
    return {
        "status": "ok",
        "keys_configured": {
            "gigachat": bool(os.getenv("GIGA_API_KEY")),
            "deepseek": bool(os.getenv("DEEPSEEK_API_KEY")),
            "openai":   bool(os.getenv("OPENAI_API_KEY")),
        },
    }