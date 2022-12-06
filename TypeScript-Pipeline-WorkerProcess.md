### TypeScript Pipeline WorkerProcess
#### Code sample notebook
---

* Taken from a pipeline based worker-process.
* Pipeline refers to a sequence of transform-streams.
* Each transform stream receives input from previous stream
  * and return a transformed output for next stream
* Process here is simply a single execution of pipeline
* Pipeline process also support
  * periodic execution
  * concurrent execution


```typescript

export interface Pipe {
  on(event: string, cb: Function): any
  pipe(stream: any): any
}

export interface Process {
  run(): Promise<void>
}

/**
 * for periodic invocation of _process.run() with _interval period
 */
export class PeriodicProcessExecutor {

  public constructor(
    private readonly _interval: number,
    private readonly _process: Process
  ) { }

  // starts the periodic invocation of _process.run()
  // use setTimeout
  public start(): void {
    const that = this
    return void setTimeout(function go(): Promise<void> {
      try {
        return that._process.run().then(() => {
          return void setTimeout(go, that._interval)
        }, (err: any) => {
          return void setTimeout(go, that._interval)
        })
      }
      catch (ex) {
        return Promise.resolve(void setTimeout(go, that._interval))
      }
    }, that._interval)
  }

}


/**
 * pulls messages from SQS queue
 */
export class SQSListener {

  public constructor(
    private readonly _sqsConnection: SQSConnection,
    private readonly _sqsQueueUrl: string
  ) { }

  // single pull of messages from _sqsQueueUrl
  public listen(): Promise<any[]> {
    return new Promise<any[]>((resolve: ResolveFunction<any[]>, reject: RejectFunction) => {
      const options: SQSReceiveMessageOptions = {
        QueueUrl: this._sqsQueueUrl,
        MaxNumberOfMessages: SQSConstants.MAX_MESSAGES_IN_SQS_RESPONSE
      }
      return this._sqsConnection.receiveMessage(options, (err: any, sqsResponse: SQSResponse) => {
        // ... boilerplate
        return resolve(sqsResponse.Messages)
      })
    })
  }
}

/**
 * implements Process
 * can be passed to PeriodicExecutor for periodic invocation
 * invokes an SQSListener, returns a pipe made out of received messages
 */
export class SQSProcess implements Process {

  public constructor(
    private readonly _sqsListener: SQSListener,
    private readonly _processingPipeTemplate: PipeTemplate
  ) { }

  public run(): Promise<void> {
    return new Promise<void>((resolve: ResolveFunction<void>, reject: RejectFunction) => {
      return this._sqsListener.listen().then((sqsMessages) => {
        const pipe = this._processingPipeTemplate.createPipe()
        pipe.on("error", (err: any) => {
          return reject(new Error("Error in processing pipeline for received SQS messages"))
        }).on("end", () => {
          return resolve()
        })
        return from2Array.obj(sqsMessages).pipe(pipe)
      })
    }
  }
}


/**
 * Example transform stream
 * Retrieves updated Google Calendar events based on received notification
 */
export class EventsRetrieverStream implements TransformStream {

  public transform(): (workUnit: WorkUnit, enc: any, done: (err?: any) => void) => void {
    return function _transform(this: any, workUnit: WorkUnit, _enc: any, done: (err?: any) => void): void {
      // ... boilerplate
      const user: GCalSyncLicensedUserData = workUnit.user
      const calendarID = user.email
      const lastSyncTime = user.lastSyncTime
      const googleCalendarManager = GoogleCalendarManagerFactory.getManager(workUnit)
      // ... boilerplate
      
      /** retrieve google calendar events */
      return void googleCalendarManager.listCalendarEvents(
        calendarID,
        lastSyncTime,
        true
      ).then((gCalEventsList) => {
        // ... boilerplate
        const gCalNotification = workUnit.calendarNotification
        const cloudextendSyncData: CloudextendSyncDataUtils.decodeSyncData(gCalNotification.channelID)
        // ... boilerplate
        gCalEventsList.items.forEach((gCalEvent: GoogleCalendarEvent) => {
           gCalEvent.cloudextendSyncData = cloudextendSyncData
        })
        // ... boilerplate
        return void orgModel.findById(orgID, (err: any, org: OrgDocument) => {
          // ... boilerplate
          const encodedSyncOwnerEmail = cloudextendSyncData.encodedSyncOwnerEmail
          const syncOwner = org.licensedUsersMap.gcalSync[encodedSyncOwnerEmail]
          syncOwner.lastSyncTime = new Date().toISOString()
          return void orgModel.findByIdAndUpdate(orgID, {
            $set: {
              [`licensedUsersMap.gcalSync.${encodedSyncOwnerEmail}.lastSyncTime`]: syncOwner.lastSyncTime
            }
          }, { new: true }, (_err: any) => {
            // transform input workunit
            workUnit.calendarEvents = []
            for (const gCalEvent of gCalEventsList.items) {
              const encodedOrganizerEmail = CloudextendSyncDataUtils.encodeEmail(gCalEvent.organizer.email)
              const isOrganizerLicensed = !!org.licensedUsersMap.gcalSync[encodedOrganizerEmail]
              gCalEvent.cloudextendSyncData!.isOrganizerLicensed = isOrganizerLicensed
              workUnit.calendarEvents.push(gCalEvent)
            }
            // output transformed workunit
            this.push(workUnit)
            return done()
          })
        })
      })
    }
  }
}

                             

/**
 * Example driver setup
 * pipeline process of streams
 * periodic execution with parametrized concurrency
 */
(async function main() {
  // ... boilerplate

  // streams
  const workUnitStream = new WorkUnitStream()
  const userDetailsRetrieverStream = new UserDetailsRetrieverStream()
  const eventProcessorStream = new EventProcessorStream()
  const notificationRemoverStream = new NotificationRemoverStream(sqsConnection, queueUrl)

  // pipeline
  const processingPipeTemplate = new PipeTemplate([
    workUnitStream,
    userDetailsRetrieverStream,
    eventProcessorStream,
    notificationRemoverStream
  ])

  // ... boilerplate

  // pipeline process
  const sqsListener = new SQSListener(sqsConnection, queueUrl)
  const sqsProcess = new SQSProcess(sqsListener, processingPipeTemplate)

  // ... boilerplate
  
  // periodic execution
  const executor = new PeriodicProcessExecutor(sqsPollingTime, sqsProcess)
  executor.start()

  // ... boilerplate

})()
```

