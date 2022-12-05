```typescript

const plugins: Restify.RequestHandler[] = [
  userDocumentPopulator,
  orgDocumentPopulator,
  gcalSyncLicenseValidator,
  getGCalOAuthTokenRefresherPlugin({
    clientID: nconf.get(Constants.CONF_GOOGLE_OAUTH_CLIENT_ID),
    refreshEndPoint: nconf.get(Constants.CONF_GOOGLE_OAUTH_REFRESH_ENDPOINT),
    clientSecret: nconf.get(Constants.CONF_GOOGLE_OAUTH_CLIENT_SECRET)
  })
];


export = (router: Restify.Server) => {
  router.get("/calendar/:calendarID/event/:calendarEventID", plugins.concat(CalendarRoutes.getCalendarEvent));
  router.get("/calendar/:id/events", plugins.concat(CalendarRoutes.listCalendarEvents));
  router.post("/calendar/:id/events/watch", plugins.concat(CalendarRoutes.watchCalendarEvents));
  router.post("/calendar/:id/channels/stop", plugins.concat(CalendarRoutes.stopCalendarChannel));
};


class CalendarRoutes {

  public static getCalendarEvent(req: Restify.Request, res: Restify.Response, next: Restify.Next): Promise<void> {

    const calendarID: string = req.params.calendarID;
    const calendarEventID: string = req.params.calendarEventID;

    if (!calendarID) {
      log.error("Google calendar id missing in req", req.url);
      return Promise.resolve(next(new Restify.BadRequestError("Google calendar id missing in req")));
    }

    if (!calendarEventID) {
      log.error("Google calendar event id missing in req", req.url);
      return Promise.resolve(next(new Restify.BadRequestError("Google calendar event id missing in req")));
    }

    // tslint:disable-next-line:max-line-length
    log.info("Received request for retrieving google calendar event with id %s for calendar with id %s", calendarEventID, calendarID);

    let calendarEventManager: GoogleCalendarEventManager;

    try {
      calendarEventManager = GoogleCalendarManagerFactory.getCalendarEventManager(req);
    }
    catch (ex) {

      log.error("Failed to create google calendar event manager for req", req.url, ex);
      return Promise.resolve(
        next(new Restify.InternalServerError("Failed to create google calendar event manager for req"))
      );

    }

    return calendarEventManager.getCalendarEvent(calendarEventID, calendarID)
    .then((calendarEvent: googleAPIs.CalendarEvent) => {

      // tslint:disable-next-line:max-line-length
      log.info("Final response for retrieving google calendar event with id %s for calendar with id %s", calendarEventID, calendarID, calendarEvent);
      res.send(200, calendarEvent);

      return next();

    }, (err: Restify.DefiniteHttpError | Error) => {

      // tslint:disable-next-line:max-line-length
      log.error("Failed to retrieve google calendar event with id %s for calendar with id %s", calendarEventID, calendarID, err);
      return next(err);

    });

  }

  public static listCalendarEvents(req: Restify.Request, res: Restify.Response, next: Restify.Next): Promise<void> {

    const calendarID: string = req.params.id;
    const updatedMin = req.query.updatedMin;

    if (!calendarID) {
      log.error("Google calendar id missing in req", req.url);
      return Promise.resolve(next(new Restify.BadRequestError("Google calendar id missing in req")));
    }

    if (!updatedMin) {
      log.error("Google calendar updated min missing in req", req.url);
      return Promise.resolve(next(new Restify.BadRequestError("Google calendar updated min missing in req")));
    }

    const showDeleted = req.query.showDeleted === "true";

    // tslint:disable-next-line:max-line-length
    log.info("Received request for retrieving list of google calendar events for calendar with id %s and with updated min %s", calendarID, updatedMin);

    let calendarEventManager: GoogleCalendarEventManager;

    try {
      calendarEventManager = GoogleCalendarManagerFactory.getCalendarEventManager(req);
    }
    catch (ex) {

      log.error("Failed to create google calendar event manager for req", req.url, ex);
      return Promise.resolve(
        next(new Restify.InternalServerError("Failed to create google calendar event manager for req"))
      );

    }

    return calendarEventManager.listCalendarEvents(calendarID, updatedMin, showDeleted)
    .then((calendarEventsList: googleAPIs.CalendarEventsList) => {

      // tslint:disable-next-line:max-line-length
      log.info("Final response for retrieving list of google calendar events for calendar with id %s and with updated min %s", calendarID, updatedMin, calendarEventsList);
      res.send(200, calendarEventsList);

      return next();

    }, (err: Restify.DefiniteHttpError | Error) => {

      // tslint:disable-next-line:max-line-length
      log.error("Failed to retrieve list of google calendar events for calendar with id %s and with updated min %s", calendarID, updatedMin, err);
      return next(err);

    });

  }

  public static watchCalendarEvents(req: Restify.Request, res: Restify.Response, next: Restify.Next): Promise<void> {

    const calendarID: string = req.params.id;
    const channelID: string = req.body.channelID;

    if (!calendarID) {
      log.error("Google calendar id missing in req", req.url);
      return Promise.resolve(next(new Restify.BadRequestError("Google calendar id missing in req")));
    }

    if (!channelID) {
      log.error("Google calendar channel id missing in req", req.url);
      return Promise.resolve(next(new Restify.BadRequestError("Google calendar channel id missing in req")));
    }

    // tslint:disable-next-line:max-line-length
    log.info("Received request for initiating watch for google calendar events for calendar with id %s through channel with id %s", calendarID, channelID);

    let calendarEventManager: GoogleCalendarEventManager;

    try {
      calendarEventManager = GoogleCalendarManagerFactory.getCalendarEventManager(req);
    }
    catch (ex) {

      log.error("Failed to create google calendar event manager for req", req.url, ex);
      return Promise.resolve(
        next(new Restify.InternalServerError("Failed to create google calendar event manager for req"))
      );

    }

    return calendarEventManager.watchCalendarEvents(calendarID, channelID)
    .then((watchedResource: GCalSyncLicensedUserData) => {

      // tslint:disable-next-line:max-line-length
      log.info("Final response for initiating watch for google calendar events for calendar with id %s through channel with id %s", calendarID, channelID, watchedResource);
      res.send(200, watchedResource);

      return next();


    }, (err: Restify.DefiniteHttpError | Error) => {

      // tslint:disable-next-line:max-line-length
      log.error("Failed to initiate watch for google calendar events for calendar with id %s through channel with id %s", calendarID, channelID, err);
      return next(err);

    });

  }

  public static stopCalendarChannel(req: Restify.Request, res: Restify.Response, next: Restify.Next): Promise<void> {

    const calendarID: string = req.params.id;
    const channelID: string = req.body.channelID;
    const resourceID: string = req.body.resourceID;

    if (!calendarID) {
      log.error("Google calendar id missing in req", req.url);
      return Promise.resolve(next(new Restify.BadRequestError("Google calendar id missing in req")));
    }

    if (!channelID) {
      log.error("Google calendar channel id missing in req", req.url);
      return Promise.resolve(next(new Restify.BadRequestError("Google calendar channel id missing in req")));
    }

    if (!resourceID) {
      log.error("Google calendar resource id missing in req", req.url);
      return Promise.resolve(next(new Restify.BadRequestError("Google calendar resource id missing in req")));
    }

    // tslint:disable-next-line:max-line-length
    log.info("Received request for stopping google calendar channel with id %s for calendar with id %s", channelID, calendarID);

    let calendarChannelManager: GoogleCalendarChannelManager;

    try {
      calendarChannelManager = GoogleCalendarManagerFactory.getCalendarChannelManager(req);
    }
    catch (ex) {

      log.error("Failed to create google calendar channel manager for req", req.url, ex);
      return Promise.resolve(
        next(new Restify.InternalServerError("Failed to create google calendar channel manager for req"))
      );

    }

    return calendarChannelManager.stopChannel(channelID, resourceID).then(() => {

      // tslint:disable-next-line:max-line-length
      log.info("Final response for stopping google calendar channel with id %s for calendar with id %s", channelID, calendarID);
      res.send(204);

      return next();

    }, (err: Restify.DefiniteHttpError | Error) => {

      // tslint:disable-next-line:max-line-length
      log.error("Failed to stop google calendar channel with id %s for calendar with id %s", channelID, calendarID, err);
      return next(err);

    });

  }

}

```