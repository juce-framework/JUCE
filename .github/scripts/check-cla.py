import json
import sys
import urllib.request

JSON_HEADER = {'Content-Type': 'application/json'}
TEXT_ENCODING = 'utf-8'

URL_GITHUB = 'https://api.github.com/repos'
URL_JUCE_CLA_CHECK = 'https://cla.juce.com/check'

ALLOWED_AUTHORS = set(
    'web-flow'
)


class NoAuthorsException(Exception):
    __MESSAGE = "No author or committer user IDs contained within commit information."
    
    def __init__(self, commits: list):
        message = f'{self.__MESSAGE}\n\n{commits}'
        super().__init__(message)


class UnsignedAuthorsException(Exception):
    __MESSAGE_0 = "The following GitHub users need to sign the JUCE CLA:",
    __MESSAGE_1 = "Please go to https://cla.juce.com to sign the JUCE Contributor Licence Agreement."
    
    def __init__(self, authors: set[str]):
        authors_str = ', '.join(authors)
        message = f'{self.__MESSAGE_0} {authors_str}\n\n{self.__MESSAGE_1}'
        super().__init__(message)


def _json_request(url: str, data: dict | None = None) -> dict:
    if data is not None:
        data = json.dumps(data).encode(TEXT_ENCODING)
    request = urllib.request.Request(url, data, JSON_HEADER, JSON_HEADER)
    with urllib.request.urlopen(request) as response:
        raw_response = response.read().decode(TEXT_ENCODING)
    return json.loads(raw_response)


def _get_authors(github_repository: str, github_event_number: int) -> set[str]:
    pr_commit_list = _json_request(f'{URL_GITHUB}/{github_repository}/pulls/{github_event_number}/commits')
    authors = set()
    for commit in pr_commit_list:
        for author_type in ['author', 'committer']:
            if author_type in commit:
                authors.add(commit[author_type]['login'])
    return authors - ALLOWED_AUTHORS


def _verify_authors(authors: set[str]) -> None:
    if not authors:
        raise NoAuthorsException()
    verification = _json_request(URL_JUCE_CLA_CHECK, {'logins': list(authors)})
    if verification['unsigned']:
        raise UnsignedAuthorsException(verification['unsigned'])


def main(github_repository: str, github_event_number: int):
    authors = _get_authors(github_repository, github_event_number)
    try:
        _verify_authors(authors)
    except Exception as e:
        print(e.message)


if __name__ == '__main__':
    github_repository = sys.argv[1]
    github_event_number = int(sys.argv[2])
    main(github_repository, github_event_number)