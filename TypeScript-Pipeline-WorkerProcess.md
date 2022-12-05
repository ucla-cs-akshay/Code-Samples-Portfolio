```typescript

export interface Pipe {
  on(event: string, cb: Function): any;
  pipe(stream: any): any;
}

export interface Process {
  run(): Promise<void>;
}

export class PeriodicProcessExecutor {

  public constructor(
    private readonly _interval: number,
    private readonly _process: Process
  ) { }

  public start(): void {
    const that = this;
    return void setTimeout(function go(): Promise<void> {
      try {
        return that._process.run().then(() => {
          return void setTimeout(go, that._interval);
        }, (err: any) => {
          return void setTimeout(go, that._interval);
        });
      }
      catch (ex) {
        return Promise.resolve(void setTimeout(go, that._interval));
      }
    }, that._interval);
  }

}


```

```typescript
export class SQSListener {

  public constructor(
    private readonly _sqsConnection: SQSConnection,
    private readonly _sqsQueueUrl: string
  ) { }

  public listen(): Promise<any[]> {

    return new Promise<any[]>((resolve: ResolveFunction<any[]>, reject: RejectFunction) => {
      
      const options: SQSReceiveMessageOptions = {
        QueueUrl: this._sqsQueueUrl,
        MaxNumberOfMessages: SQSConstants.MAX_MESSAGES_IN_SQS_RESPONSE
      };

      return this._sqsConnection.receiveMessage(options, (err: any, sqsResponse: SQSResponse) => {
        if (err) {
          return reject(new Error("Failed to receive SQS messages for provided options"));
        }
        return resolve(sqsResponse.Messages);
      });

    });

  }

}
```

```typescript
export class SQSProcess implements Process {

  public constructor(
    private readonly _sqsListener: SQSListener,
    private readonly _processingPipeTemplate: PipeTemplate
  ) { }

  public run(): Promise<void> {

    return new Promise<void>((resolve: ResolveFunction<void>, reject: RejectFunction) => {

      return this._sqsListener.listen().then((sqsMessages) => {

        const pipe = this._processingPipeTemplate.createPipe();

        pipe.on("error", (err: any) => {
          return reject(new Error("Error in processing pipeline for received SQS messages"));
        }).on("end", () => {
          return resolve();
        });

        return from2Array.obj(sqsMessages).pipe(pipe);

      })

    }

}
```


```typscript

const main = async () => {

  if (["prod", "staging"].indexOf(environment) > -1) {
    nconf.argv().env();
    log.info("Fetching keys from vault since %s environment", environment);
    nconf.use("memory");
    await commons.Vault.load(
      process.env.NODE_ENV!,
      process.env.VAULT_PATH!,
      process.env.VAULT_REGION!,
      require("../conf/keys.json"),
      nconf
    );
    log.info("Successfully fetched keys from vault %s", process.env.VAULT_PATH);
  } else {
    log.info("Loading keys from local config file for %s environment", environment);
    // Configure nconf
    const pathToConfig = `../conf/${environment}.json`;
    log.info("Path to configs ", path.resolve(__dirname, pathToConfig));
    nconf.argv().env().file(path.resolve(__dirname, pathToConfig));
  }

  const {
    EventProcessorStream, NotificationRemoverStream, UserDetailsRetrieverStream,
    WorkUnitStream, sqsConnection
  } = require("./lib");


  // Create streams that will process the SQS messages
  const workUnitStream = new WorkUnitStream();
  const userDetailsRetrieverStream = new UserDetailsRetrieverStream();
  const eventProcessorStream = new EventProcessorStream();
  const notificationRemoverStream = new NotificationRemoverStream(sqsConnection, queueUrl);
  const processingPipeTemplate = new PipeTemplate([
    workUnitStream,
    userDetailsRetrieverStream,
    eventProcessorStream,
    notificationRemoverStream
  ]);

  // Create the SQS listener which will periodically poll the SQS Queue
  const sqsListener = new SQSListener(sqsConnection, queueUrl);
  const sqsProcess = new SQSProcess(sqsListener, processingPipeTemplate);


  void mongoose.connect(mongoDBURL, connectOptions, (error: any) => {

    if (error) {
      return process.exit(-1);
    }

    const executor = new PeriodicProcessExecutor(sqsPollingTime, sqsProcess);
    executor.start();

  });

};

void main();
```

