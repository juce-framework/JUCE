from configure_logger import configure_logger

from logging import getLogger
from urllib.request import Request, urlopen
from urllib.error import HTTPError, URLError
from json import dumps, loads
from os import environ
from shutil import copyfileobj
from time import sleep

logger = getLogger(__name__)
configure_logger(logger)

def github_api_request(path, method='GET', data=None):
    url = f'https://api.github.com/repos/{environ["GITHUB_REPOSITORY"]}/{path}'
    logger.debug(f'Requesting GitHub API: {url}')
    serialised_data = dumps(data).encode('utf-8') if data else None
    if serialised_data:
        logger.debug(f'Data: {serialised_data}')
    req = Request(
        url=url,
        method=method,
        headers={
          'Accept': 'application/vnd.github+json',
          'X-GitHub-Api-Version': '2022-11-28'
        },
        data=serialised_data
    )
    req.add_unredirected_header('Authorization', f'Bearer {environ["GITHUB_API_TOKEN"]}')
    num_attempts = 0
    while True:
        response = None
        try:
            response = urlopen(req)
            return response
        except (HTTPError, URLError) as e:
            num_attempts += 1
            if num_attempts == 3:
                logger.warning(f'GitHub API access failed\n{e.reason}\n{e.headers}\n{e.fp.read()}')
                raise e
            logger.debug(f'Request attempt {num_attempts} failed, retrying')
            sleep(10)

def json_github_api_request(path, method='GET', data=None):
    with github_api_request(path, method, data) as response:
        result = loads(response.read().decode('utf-8'))
    logger.debug(f'GitHub API result: {result}')
    return result

def download_github_api_request(filename, path, method='GET', data=None):
    with github_api_request(path, method, data) as response:
        with open(filename, 'wb') as f:
            copyfileobj(response, f)
    logger.debug(f'Downloaded to: {filename}')

