import logging
from os import getenv
from sys import stdout

def configure_logger(logger):
    handler = logging.StreamHandler(stdout)
    formatter = logging.Formatter('[%(name)s] %(message)s')
    handler.setFormatter(formatter)
    level = logging.DEBUG if (getenv('RUNNER_DEBUG', '0').lower() not in ('0', 'f', 'false')) else logging.INFO
    logger.setLevel(level)
    handler.setLevel(level)
    logger.addHandler(handler)

