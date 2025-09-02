from configure_logger import configure_logger
from github_api_request import github_api_request, json_github_api_request

from logging import getLogger
from os import getenv
from json import loads, dumps

logger = getLogger(__name__)
configure_logger(logger)

input_string = getenv('TRIGGER_WORKFLOW_INPUTS', '{}')
logger.debug(f'Input variable: {input_string}')
input_json = loads(input_string)
for key, value in input_json.items():
    if not isinstance(value, str):
        input_json[key] = dumps(value)
logger.debug(f'Stringified input: {input_json}')

api_path_prefix = 'actions/workflows'

workflows = json_github_api_request(api_path_prefix)
workflow_path = getenv('TRIGGER_WORKFLOW_PATH',
                       '.github/workflows/juce_private_build.yml')
workflow = [x for x in workflows['workflows'] if x['path'] == workflow_path][0]
logger.debug(f'Workflow: {workflow}')

trigger_data = {
    'ref': getenv('TRIGGER_WORKFLOW_REF', getenv('GITHUB_REF_NAME')),
    'inputs': input_json
}
logger.debug(f'Trigger_data: {trigger_data}')
github_api_request(f'{api_path_prefix}/{workflow["id"]}/dispatches',
                   method='POST',
                   data=trigger_data)

