# UpdateClient

The Update client is a daemon, that runs in the background and is responsible for updating the CamClient application.

## Message order

1. The client sends a version request to the server, asking for the current server version of the source code.
2. The client sends a server token request, to get a generated server token for authentication.
3. If the local version and the server version does not match, an update begin request is sent.
4. If an update request was send, we will now receive the update in small message pieces.
5. If the update is finished, the updater starts the client application and goes into an idle mode
6. If no update was received, the updater starts the client application and goes into an idle mode
7. The idle mode does only receive server updates, in this way the server can initiate an update as well

