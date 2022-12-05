## MVC Angular Web Application

```
• JavaScript  • Angular.js  • Google Chrome WebExtension SDK
```
---

### Overview

Angular.js web application. Most logic is client-side, including routing.
  * MVC architecture

The Google Chrome WebExtension injects the app into the Gmail webpage,
as a sidebar WebPlugin. Also coordinates asynchronous messages between :-
  * Gmail webpage
  * App
  * NetSuite iframe


### Directory Structure

```
.
├── app
│   ├── controllers
|   |       • Angular.js UI controllers
|   |           • component state, event handlers, user interactions
│   ├── directives
|   |       • Angular.js directives (onLoad, onResize)
│   ├── filters
|   |       • Angular.js filters
│   ├── index.html
|   |       • main container html file
│   ├── index.js
|   |       • main / entrypoint
│   ├── services
|	|		• business logic services
|	|		• app wide background singleton services
│   └── views
|			• component templates
└── webextension
        • webextension background logic
```


```javascript
/* global jQuery*/
'use strict';

angular.module( 'core' ).controller( 'RecordController', [ '$q', 'LogService', 'NotificationService',
    'UserStateService', 'UtilityService', 'NetsuiteSessionService', 'RecordService', 'LoadingService',
    'StateTransitionService', 'LicenseService', 'EmailAttacherService', 'NetsuiteBundleService', 'NateroService',
    function ( $q, LogService, NotificationService, UserStateService, UtilityService, NetsuiteSessionService,
        RecordService, LoadingService, StateTransitionService, LicenseService, EmailAttacherService,
        NetsuiteBundleService, NateroService
    ) {
        LogService.debug( 'RecordController initialized.' ).print();

        var messageId = null,
            stateParams = StateTransitionService.getCurrentStateParams(),
            recordsList = stateParams.recordsList,
            index = stateParams.index,
            type = stateParams.type,
            that = this;

        that.hasNoMessageInContext = false; // initialised to false to keep UI stabilized while the view loads
        that.netsuiteBaseUrl = null;
        that.includeAttachmentsByDefault = false;
        that.attachmentsMapModel = {};
        that.autoCreateMissingSenderRecipientContacts = true;
        that.bundleSidePreferences = null;
        that.processedRecord = {};
        that.messageAttachments = [];
        that.messageAttachmentsPanelExpended = false;
        that.recordToDisplay = {
            'standard': [],
            'custom': []
        };
        that.metadataNotSupported = false;

        function getVisibleFields ( pRecord ) {
            var recordToReturn = [];

            Object.keys( pRecord ).forEach( function ( key ) {
                var field = pRecord[key];

                if ( field.isVisible ) {
                    recordToReturn.push( field );
                }
            } );

            return recordToReturn;
        }

        that.getSelectedMsgAttachmentsCount = function () {
            var attachmentsMap = EmailAttacherService.getAttachmentsMap(),
                attachmentsMapKeys = Object.keys( attachmentsMap );

            return Object.keys( attachmentsMapKeys.reduce( function ( map, item ) {
                map[ attachmentsMap[item].attachmentId ] = attachmentsMap[item];
                return map;
            }, {} ) ).length;
        };

        that.nextRecord = function () {
            return StateTransitionService.goTo( 'records', {
                'recordsList': recordsList,
                'type': type,
                'index': index + 1
            }, {
                'reload': true,
                'loadingMessage': 'Loading the record details...'
            } );
        };

        that.previousRecord = function () {
            return StateTransitionService.goTo( 'records', {
                'recordsList': recordsList,
                'type': type,
                'index': index - 1
            }, {
                'reload': true,
                'loadingMessage': 'Loading the record details...'
            } );
        };

        that.isFromHistory = function () {
            return type === 'history';
        };

        that.goBack = function () {
            if ( type === 'history' ) {
                return StateTransitionService.goTo( 'history' );
            }
            else if ( type === 'create' ) {
                return StateTransitionService.goTo( 'create' );
            }

            return StateTransitionService.goTo( 'home' );
        };

        that.openFilter = function () {
            return StateTransitionService.goTo( 'fieldFilter', {
                'recordsList': recordsList,
                'type': type,
                'index': index
            }, {
                'loadingMessage': 'Loading your filter details...'
            } );
        };

        that.openSenderRecipientEntitiesPage = function () {
            if ( that.hasNoMessageInContext ) {
                LogService.error( 'No meesage in context' ).print();
                return NotificationService.notifyError( 'Please open an email first..' );
            }

            StateTransitionService.goTo( 'senderRecipientEntities', {
                'from': 'records',
                'recordsList': recordsList,
                'type': type,
                'index': index
            }, {
                'loadingMessage': 'Loading Sender/Recipient entities selection page...'
            } );
        };

        that.openEdit = function () {
            return StateTransitionService.goTo( 'edit', {
                'recordsList': recordsList,
                'type': type,
                'index': index
            } );
        };

        that.getPageInfo = function () {
            return 'Record ' + ( index + 1 ) + ' of ' + recordsList.length;
        };

        that.isLeftArrowVisible = function ( ) {
            return index !== 0;
        };

        that.isRightArrowVisible = function ( ) {
            return index !== ( recordsList.length - 1 );
        };

        that.isSMFButtonVisible = function () {
            return (
                !(
                    UtilityService.isArrayEmpty( that.processedRecord.standard ) &&
                    UtilityService.isArrayEmpty( that.processedRecord.custom )
                ) &&
                !(
                    UtilityService.isArrayEmpty( that.recordToDisplay.standard ) &&
                    UtilityService.isArrayEmpty( that.recordToDisplay.custom )
                ) &&
                (
                    that.recordToDisplay.standard.length +
                    that.recordToDisplay.custom.length <= 10
                ) &&
                (
                    that.processedRecord.standard.length +
                    that.processedRecord.custom.length > 10
                )
            );
        };
        that.noMessageAttachments = function () {
            return ( that.messageAttachments && that.messageAttachments.length === 0 );
        };
        that.addToAttachments = function ( attachment ) {
            EmailAttacherService.addToAttachmentsMap( attachment );
            that.includeAttachmentsByDefault = ( that.getSelectedMsgAttachmentsCount() === that.messageAttachments.length );
        };
        that.setAttachmentsCount = function ( number ) {
            EmailAttacherService.setAttachmentsCount( number );
        };
        that.getAttachmentsCount = function () {
            return EmailAttacherService.getAttachmentsCount();
        };
        that.setIncludeattachmentsStatus = function ( number ) {
            if ( number === 0 ) {
                that.includeAttachmentsByDefault = false;
            }
        };
        that.updateAllAttachments = function () {
            var val = that.includeAttachmentsByDefault;

            if ( val === true ) {
                that.messageAttachments.forEach( function ( attachment ) {
                    EmailAttacherService.addAttachment( attachment );
                    that.attachmentsMapModel[attachment.attachmentId] = val;
                } );
            }
            else {
                that.messageAttachments.forEach( function ( attachment ) {
                    EmailAttacherService.removeAttachment( attachment );
                    that.attachmentsMapModel[attachment.attachmentId] = val;
                } );
            }
            that.noMessageAttachments = function () {
                return ( that.messageAttachments && that.messageAttachments.length === 0 );
            };
        };


        that.messageAttachmentsPanelClicked = function () {
            if ( that.isFromHistory() || that.noMessageAttachments() ) {
                return;
            }
            that.messageAttachmentsPanelExpended = !that.messageAttachmentsPanelExpended;
            that.adjustAttachmentPanel();
        };

        that.adjustAttachmentPanel = function () {
            if ( that.isFromHistory() ) {
                return;
            }
            var attachmentHeight = 30;
            var collapsedAttachHeight = 30;

            if ( that.messageAttachments.length === 1 ) {
                attachmentHeight = 46;
            }
            else if ( that.messageAttachments.length === 2 ) {
                attachmentHeight = 93;
            }
            else {
                attachmentHeight = 138;
            }
            if ( that.messageAttachmentsPanelExpended ) {

                jQuery( '.include-attachement-2' ).css( 'height', attachmentHeight + collapsedAttachHeight + 'px' );
                jQuery( '.attachement-box' ).css( 'height', attachmentHeight + collapsedAttachHeight + 'px' );
                jQuery( '.attachement-list' ).css( 'height', attachmentHeight + 'px' );

                jQuery( '.bx-pager' ).css( 'bottom', attachmentHeight + 'px' );

                jQuery( '.bx-controls-direction' ).css( 'bottom', attachmentHeight + 'px' );
            }
            else {
                jQuery( '.include-attachement-2' ).css( 'height', collapsedAttachHeight + 'px' );
                jQuery( '.attachement-box' ).css( 'height', collapsedAttachHeight + 'px' );
                jQuery( '.attachement-list' ).css( 'height', 0 + 'px' );

                jQuery( '.bx-pager' ).css( 'bottom', 0 + 'px' );

                jQuery( '.bx-controls-direction' ).css( 'bottom', 0 + 'px' );
            }
        };
        that.attachMessage = function () {
            EmailAttacherService.addToAttachMap( {
                'recordType': that.recordToDisplay.recordType,
                'recordId': that.recordToDisplay.recordId,
                'recordTypeId': that.recordToDisplay.recordTypeId,
                'recordTypeName': that.recordToDisplay.recordTypeName,
                'recordName': that.recordToDisplay.recordName || '',
                'recordUrl': that.recordToDisplay.recordUrl
            }, true );

            return EmailAttacherService.attachEmail( {
                'includeAttachments': that.includeAttachmentsByDefault,
                'autoCreateMissingSenderRecipientContacts': that.autoCreateMissingSenderRecipientContacts,
                'hndlMsngSndrRcpntEntts': that.bundleSidePreferences.hndlMsngSndrRcpntEntts
            } )
            .then( function () {
                return EmailAttacherService.getMessageIdInContext()
                .then( function ( currentMessageId ) {
                    that.hasNoMessageInContext = false;
                    if ( messageId !== currentMessageId ) {
                        StateTransitionService.goTo( 'welcome' );
                    }
                    that.updateAllAttachments();
                } )
                .catch( function ( error ) {
                    if ( error === 'no_msgid_in_context' ) {
                        that.hasNoMessageInContext = true;
                        StateTransitionService.goTo( 'welcome' );
                        that.updateAllAttachments();
                    }
                } );
            } )
            .catch( function ( error ) {
                LogService.error( 'Unable to attach the email:', error ).print();
                that.updateAllAttachments();
            } )
            .then( LoadingService.unload );
        };


        ( function runRecordController ( record ) {
            LicenseService.validateUserLicense()
            .then( NetsuiteSessionService.validateCurrentNetsuiteSession )
            .then( function () {
                that.recordToDisplay.recordTypeName = record.recordTypeName;
                that.netsuiteBaseUrl = NetsuiteSessionService.getNSBaseUrl();
                that.netsuiteBaseUrl = that.netsuiteBaseUrl && that.netsuiteBaseUrl.replace( /\/$/, '' );

                return $q.allSettled( {
                    'getSettingIncludeAttachments': UserStateService.getSetting( 'includeAttachmentsByDefault' ),
                    'getSettingAutoCreateMissingSenderRecipientContacts': UserStateService.getSetting( 'autoCreateMissingSenderRecipientContacts' ),
                    'getBundleSidePreferences': NetsuiteBundleService.getPreferencesFromNS(),
                    'getProcessedRecord': RecordService.getProcessedRecord( record ),
                    'getMessageIdInContext': EmailAttacherService.getMessageIdInContext()
                } )
                .then( function ( results ) {
                    if ( results.getSettingIncludeAttachments.status === 'rejected' ) {
                        return NotificationService.notifyError( results.getSettingIncludeAttachments.value.toString() );
                    }

                    if ( results.getSettingAutoCreateMissingSenderRecipientContacts.status === 'rejected' ) {
                        return NotificationService.notifyError( results.getSettingAutoCreateMissingSenderRecipientContacts.value.toString() );
                    }

                    if ( results.getBundleSidePreferences.status === 'rejected' ) {
                        return NotificationService.notifyError( results.getBundleSidePreferences.value.toString() );
                    }

                    if ( results.getProcessedRecord.status === 'rejected' ) {
                        var error = results.getProcessedRecord.value;

                        if ( error.code === 'CEHE-00035' ) { return; }

                        NotificationService.notifyError( error.toString() );

                        if ( error.code === 'CEHE-00042' ) { return; }

                        return that.goBack();
                    }

                    if ( results.getMessageIdInContext.status === 'resolved' ) {
                        messageId = results.getMessageIdInContext.value;
                        that.hasNoMessageInContext = false;
                    }
                    else {
                        messageId = null;
                        that.hasNoMessageInContext = true;
                    }

                    that.includeAttachmentsByDefault = !!results.getSettingIncludeAttachments.value;
                    that.autoCreateMissingSenderRecipientContacts = !!results.getSettingAutoCreateMissingSenderRecipientContacts.value;
                    that.bundleSidePreferences = results.getBundleSidePreferences.value;
                    that.autoCreateMissingSenderRecipientContacts =
                        ( !that.bundleSidePreferences.hndlMsngSndrRcpntEntts || that.bundleSidePreferences.hndlMsngSndrRcpntEntts === '1' ) ?
                            that.autoCreateMissingSenderRecipientContacts : false;

                    var pRecord = results.getProcessedRecord.value,
                        metadataNotSupportedRecords = [
                            'creditcardcharge',
                            'othername'
                        ];

                    that.processedRecord = pRecord;

                    that.recordToDisplay.recordTypeName = pRecord.recordTypeName;
                    that.recordToDisplay.recordUrl = pRecord.recordUrl;
                    that.recordToDisplay.recordId = pRecord.recordId;
                    that.recordToDisplay.recordTypeId = pRecord.recordTypeId;
                    that.recordToDisplay.recordType = pRecord.recordType;
                    that.recordToDisplay.recordName = pRecord.recordName;

                    that.recordToDisplay.standard = getVisibleFields( pRecord.standard );
                    that.recordToDisplay.custom = getVisibleFields( pRecord.custom );

                    that.metadataNotSupported = metadataNotSupportedRecords
                        .indexOf( pRecord.recordTypeId.toLowerCase() ) !== -1;

                    NateroService.logEvent( {
                        'action': 'feature',
                        'module': 'data-operations',
                        'feature': 'view-record',
                        'createdAt': Date.now(),
                        'details': {
                            'recordType': record.recordType
                        }
                    } );

                    EmailAttacherService.getMessageInContext().then( function ( message ) {
                        console.log( 'message is :', message );
                        that.messageAttachments = message && message.attachments;
                        EmailAttacherService.resetAttachments();
                        that.messageAttachments.forEach( function ( attachment ) {
                            that.attachmentsMapModel[attachment.attachmentId] = that.includeAttachmentsByDefault;
                        } );
                        that.updateAllAttachments();
                        that.setAttachmentsCount( that.messageAttachments.length );
                        that.setIncludeattachmentsStatus( that.messageAttachments.length );

                    } );
                } );
            } )
            .catch( function ( error ) {
                LogService.error( error.toString(), error ).print();
                NotificationService.notifyError( error.toString() );

                if ( error.code === 'CEHE-00010' || error.code === 'CEHE-00011' || error.code === 'CEHE-00049' ) {
                    StateTransitionService.goTo( 'nsSession' );
                }
            } )
            .then( LoadingService.unload );
        } )( recordsList[index] );
    }
] );```