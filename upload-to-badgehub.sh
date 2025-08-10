LOCAL_ELF_NAME=test_badghub_client.elf

#BADGEHUB_SAMPLE_APP_API_TOKEN=TODO_EXPORT_IN_ENV_MANUALLY
BADGEHUB_PROJECT_SLUG=test_badge_hub_client
REMOTE_ELF_NAME=test_badge_hub_client.elf

curl -X POST -H "badgehub-api-token: ${BADGEHUB_SAMPLE_APP_API_TOKEN}" \
-F "file=@./build/app_elfs/${LOCAL_ELF_NAME}" \
https://badge.why2025.org/api/v3/projects/${BADGEHUB_PROJECT_SLUG}/draft/files/${REMOTE_ELF_NAME} \
&& echo "did upload badgehub.elf to project ${BADGEHUB_PROJECT_SLUG}" && \
curl -X 'PATCH' -H "badgehub-api-token: ${BADGEHUB_SAMPLE_APP_API_TOKEN}" \
  "https://badge.why2025.org/api/v3/projects/${BADGEHUB_PROJECT_SLUG}/publish" \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
&& echo "did publish"
