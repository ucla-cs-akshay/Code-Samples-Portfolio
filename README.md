## Code Samples / Portfolio

#### AKSHAY SINGHAL
  * https://www.linkedin.com/in/akshaysinghal014
  * akshaysinghal@cs.ucla.edu
  * (424) 448-6308  —  Los Angeles, CA
---


### Abstract

This is a portfolio of my code samples, taken from the professional projects
where I have personally led the development and engineering effort,
over my 5+ years of industry experience in Full-Stack Cross-Platform Development.

Best to start with the `README.md` file, which describes the directory structure
& constituent files, and brief helpful comments.

### Curation Note

To keep this portfolio quickly-browsable and high-signal for the reviewer, I have tried to include
ONLY specific bits-and-pieces of the projects, and excluded all boilerplate code. I have also
renamed and restructured files and folders for easier mental association with conventional/typical
software artifacts.

Kind request to the reviewer to please ping me for full project code, and/or detailed discussion
about my specific design / engineering choices such as my approach / intention / thinking.

###  

```
```

### Directory structure

```
.
├── Backend
│   |   • contains code samples from backend modules, e.g.
│   |       • mongodb orm for persistence / query
│   |       • url-path-based routers
│   |       • api route handlers / controllers
│   |       • business logic services
│   |       • 3rd party oauth (google, netsuite)
│   |       • aws sdk wrappers (sqs, ses)
│   |       • pipeline, streams, worker process
│   |
│   ├── MVC-ApplicationServer
│   |       • typescript, node.js, express.js, mongodb, mongoose
│   |       • application server for angular web application
│   |       • 3rd party oauth logic (google, netsuite)
│   |       • proxy for gmail rest api and netsuite soap webservice
│   |       
│   ├── Pipeline-WorkerProcess
│   |       • typescript, node.js, aws sdk, mongodb, mongoose
│   |       • streams and pipeline based worker process
│   |       • takes work units 1-by-1 off sqs message queue
│   |       • processes the work unit through a pipeline of transform streams
│   |       
│   ├── REST-WebService-01
│   |       • typescript, node.js, restify, mongodb, mongoose
│   |       • rest based web service
│   |       • proxy for google calendar rest api
│   |
│   ├── REST-WebService-02
│   |       • typescript, node.js, restify, mongodb, mongoose
│   |       • REST based web service
│   |       • proxy for netsuite soap webservice
│   |
│   └── Webhook
│           • typescript, node.js, restify, aws sdk mongodb, mongoose
│           • webhook for update notifications from google calendar
│           • prepares work unit and enqueues into sqs message queue
│
│
└── Frontend
    |   • contains code samples from frontend applications, e.g.
    |       • ui components / templates
    |       • controllers, component state
    |       • services
    |           • application state
    |           • models
    |           • data fetching & backend rpc
    |           • client-side routing
    |           • caching, logging
    |
    ├── MVC-Angular-WebApplication
    |       • javascript, angular.js, google chrome webextension sdk
    |       • client-side heavy application (single page application, client side routing)
    |       • webextension injects the app into gmail (https://mail.google.com)
    |           • as sidebar webplugin
    |
    └── MVC-GoogleAppsScript-CrossPlatformAddon
            • typescript, google apps script / google workspace add-on framework & sdk
            • cross-platform (web, android, ios)
            • server-side rendered
                • very thin / light-weight client
```
---

### Contact

AKSHAY SINGHAL
  * https://www.linkedin.com/in/akshaysinghal014
  * akshaysinghal@cs.ucla.edu
  * (424) 448-6308  —  Los Angeles, CA