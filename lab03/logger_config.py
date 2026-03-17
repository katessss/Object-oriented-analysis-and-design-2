import logging
from colorlog import ColoredFormatter


def setup_logger(name: str = __name__, extra_handlers: list = None) -> logging.Logger:
    logger = logging.getLogger(name)
    logger.setLevel(logging.DEBUG)
    logger.propagate = False

    if logger.handlers:
        if extra_handlers:
            for h in extra_handlers:
                if h not in logger.handlers:
                    logger.addHandler(h)
        return logger

    # Консольный хендлер с цветами
    handler = logging.StreamHandler()
    handler.setLevel(logging.DEBUG)
    handler.setFormatter(ColoredFormatter(
        "%(log_color)s%(asctime)s | %(levelname)s | %(name)s:%(lineno)d | %(message)s",
        datefmt="%H:%M:%S",
        log_colors={
            "DEBUG":    "cyan",
            "INFO":     "green",
            "WARNING":  "yellow",
            "ERROR":    "red",
            "CRITICAL": "bold_red",
        },
    ))
    logger.addHandler(handler)

    if extra_handlers:
        for h in extra_handlers:
            logger.addHandler(h)

    return logger