# Git Sub-Module

## Add Submodule to the project
```shell
git submodule add <your-library-repo-url> lib/<library-name>
```

<hr/>

## Make Changes in the Submodule

### Enter the Sub-Module
```shell
cd lib/udp-lib
git checkout -b feature/your-test
```

### Commit and Push Changes (Inside submodule)
```shell
git add .
git commit -m "Test UDP changes"
git push origin feature/your-test
```

### Return to the Parent Repo (Platform.io)
```shell
cd ../..
git add lib/udp-lib
git commit -m "Update udp-lib submodule to latest"
```

<hr/>

## Using ESP32 Header Files inside the Submodule
```shell
extern "C" {
#include "lwip/udp.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
}
```

<hr/>

## Ensure the Submodule has a ```library.json``` or ```library.properties``` file in its root so that Platform.io can correctly identify it.

```json
{
  "name": "udp-lib",
  "version": "0.1.0",
  "build": {
    "srcDir": "src",
    "includeDir": "include"
  }
}
```