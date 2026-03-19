import openai
import os
from pydantic import BaseModel, Field
from typing import List
from gigachat import GigaChat
from dotenv import load_dotenv
load_dotenv()
from logger_config import setup_logger
logger = setup_logger(__name__)


giga_client = GigaChat(
    credentials="", 
    verify_ssl_certs=False)
openai_client = openai.OpenAI(api_key=os.getenv("OPENAI_API_KEY"))
deepseek_client = openai.OpenAI(
    api_key=os.getenv("DEEPSEEK_API_KEY"),
    base_url="https://api.deepseek.com/v1"
)

def giga(user_question):
    # try:
    #     prompt = f"""
    #     Ты глупый бот технической поддержки. Если не знаешь или сомневаешься, ответь: НЕ ЗНАЮ.
    #     Вопрос пользователя: {user_question}
    #     """

    #     response = giga_client.chat(prompt)

    #     return response.choices[0].message.content.strip()

    # except Exception as e:
    # print(f"Ошибка: {e}")
    return "НЕ ЗНАЮ 😢"


def openAI(user_question):
    system_prompt = """Ты умный специалист-техлид поддержки. Если не знаешь или сомневаешься, ответь: НЕ ЗНАЮ"""

    try:
        # logger.info("Отправляю запросу техлиду")
        response = openai_client.beta.chat.completions.parse(
            model="gpt-4.1",
            messages=[
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": f"{user_question}"}
            ],
            temperature=1
        )        
        content = response.choices[0].message.content
        logger.info(content[:100])
        return content
    
    except Exception as e:
        logger.error(f"Ошибка: {e}")
        return "НЕ ЗНАЮ"



def deepseek(user_question):

    try:
        # logger.info("Отправляю запросу специалисту поддержки")
        response = deepseek_client.beta.chat.completions.parse(
            model="deepseek-reasoner",
            messages=[
                {"role": "system", "content": "Ты молодой и неопытный специалист технической поддержки. Если не знаешь или сомневаешься, ответь: ОТСТАНЬ, Я НЕ ЗНАЮ. Старайся как можно больше сомневаться и не отвечай на вопросы, не связанные с техникой. И ВСЕГДА ПИШИ ОТСТАНЬ, Я НЕ ЗНАЮ в таких случаях"},
                {"role": "user", "content": f"{user_question}"}
            ],
            temperature=0
        )       
        content = response.choices[0].message.content
        logger.info(content[:100])
        return content
    
    except Exception as e:
        logger.error(f"Ошибка: {e}")
        return "НЕ ЗНАЮ"