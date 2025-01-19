/****************************************************************************
**
** Copyright (C) VCreate Logic Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

pragma Singleton

import QtQuick 2.15
import Qt.labs.settings 1.0
import QtQuick.Controls.Material 2.15

import io.scrite.components 1.0

import "qrc:/js/utils.js" as Utils
import "qrc:/qml/controls"
import "qrc:/qml/dialogs"

Item {
    id: root

    visible: false

    property var taxonomy

    function init(_parent) {
        if( !(_parent && Scrite.app.verifyType(_parent, "QQuickItem")) )
            _parent = Scrite.window.contentItem

        parent = _parent
        visible = false
        anchors.fill = parent
    }

    function planActionLinkText(plan) {
        if(!plan)
            return ""

        let ret = Utils.toTitleCase(plan.action.kind) + " »"
        if(taxonomy && taxonomy["planKinds"]) {
            const planKinds = taxonomy["planKinds"]
            for(let i=0; i<planKinds.length; i++) {
                const planKind = planKinds[i]
                if(planKind.name === plan.kind) {
                    ret = planKind.label + " »"
                    break
                }
            }
        }

        return ret
    }

    function subscribeTo(plan, callList) {
        if(!Scrite.user.loggedIn) {
            MessageBox.information("Login required", "This plan can be subscribed to only after you login.")
            return
        }

        if(plan.kind !== "trial" && !Scrite.user.info.hasTrialSubscription && !Scrite.user.info.isEarlyAdopter) {
            const buttons = [Utils.toTitleCase(plan.action.kind) + " " + plan.title, "Go Back"]
            MessageBox.question("Use Trial", "We recommend that you use the app on trial first before signing up for any other plan.",
                                buttons, (option) => {
                                    if(option === buttons[0])
                                        _private.subscribeTo(plan, callList)
                                })
        } else
            _private.subscribeTo(plan, callList)
    }

    function populateComparisonTableModel(plans, listModel) {
        // Row: Plan title
        listModel.append({
                             "kind": "label",
                             "attributes": {
                                 "text": "",
                                 "color": "" + Runtime.colors.accent.c600.text,
                                 "background": "" + Runtime.colors.accent.c600.background,
                                 "horizontalAlignment": Text.AlignLeft,
                                 "font": {
                                     "pointSize": Runtime.idealFontMetrics.font.pointSize,
                                     "weight": Font.Normal,
                                     "italic": false
                                 }
                             }
                         })

        plans.forEach( (plan) => {
                          listModel.append({
                                               "kind": "label",
                                               "attributes": {
                                                   "text": plan.title,
                                                   "color": "" + Runtime.colors.accent.c600.text,
                                                   "background": "" + Runtime.colors.accent.c600.background,
                                                   "horizontalAlignment": Text.AlignHCenter,
                                                   "font": {
                                                       "pointSize": Runtime.idealFontMetrics.font.pointSize + 2,
                                                       "weight": Font.Bold,
                                                       "italic": false
                                                   }
                                               }
                                           })
                      })

        // Row: Plan subtitle
        listModel.append({
                             "kind": "label",
                             "attributes": {
                                 "text": "",
                                 "color": "" + Runtime.colors.accent.c600.text,
                                 "background": "" + Runtime.colors.accent.c600.background,
                                 "horizontalAlignment": Text.AlignLeft,
                                 "font": {
                                     "pointSize": Runtime.idealFontMetrics.font.pointSize,
                                     "weight": Font.Normal,
                                     "italic": false
                                 }
                             }
                         })
        plans.forEach( (plan) => {
                          listModel.append({
                                               "kind": "label",
                                               "attributes": {
                                                   "text": plan.subtitle,
                                                   "color": "" + Runtime.colors.accent.c600.text,
                                                   "background": "" + Runtime.colors.accent.c600.background,
                                                   "horizontalAlignment": Text.AlignHCenter,
                                                   "font": {
                                                       "pointSize": Runtime.minimumFontMetrics.font.pointSize,
                                                       "weight": Font.Normal,
                                                       "italic": true
                                                   }
                                               }
                                           })
                      })

        // Row: Plan pricing
        listModel.append({
                             "kind": "label",
                             "attributes": {
                                 "text": "",
                                 "color": "" + Runtime.colors.accent.c600.text,
                                 "background": "" + Runtime.colors.accent.c600.background,
                                 "horizontalAlignment": Text.AlignLeft,
                                 "font": {
                                     "pointSize": Runtime.idealFontMetrics.font.pointSize,
                                     "weight": Font.Normal,
                                     "italic": false
                                 }
                             }
                         })
        plans.forEach( (plan) => {
                          listModel.append({
                                               "kind": "label",
                                               "attributes": {
                                                   "text": (plan.pricing.price === 0 ? "FREE" : (Scrite.currencySymbol(plan.pricing.currency) + " " + plan.pricing.price + " *")),
                                                   "color": "" + Runtime.colors.accent.c600.text,
                                                   "background": "" + Runtime.colors.accent.c600.background,
                                                   "horizontalAlignment": Text.AlignHCenter,
                                                   "font": {
                                                       "pointSize": Runtime.idealFontMetrics.font.pointSize,
                                                       "weight": Font.Bold,
                                                       "italic": false
                                                   }
                                               }
                                           })
                      })

        // Row: Plan get/buy links
        listModel.append({
                             "kind": "label",
                             "attributes": {
                                 "text": "",
                                 "color": "" + Runtime.colors.accent.c100.text,
                                 "background": "" + Runtime.colors.accent.c100.background,
                                 "horizontalAlignment": Text.AlignLeft,
                                 "font": {
                                     "pointSize": Runtime.idealFontMetrics.font.pointSize,
                                     "weight": Font.Normal,
                                     "italic": false
                                 }
                             }
                         })
        plans.forEach( (plan) => {
                          listModel.append({
                                               "kind": "link",
                                               "attributes": {
                                                   "text": root.planActionLinkText(plan),
                                                   "background": "" + Runtime.colors.primary.c100.background,
                                                   "horizontalAlignment": Text.AlignHCenter,
                                                   "font": {
                                                       "pointSize": Runtime.idealFontMetrics.font.pointSize,
                                                       "weight": Font.Bold,
                                                       "italic": false
                                                   },
                                                   "plan": plan
                                               }
                                           })
                      })

        // Row: Plan duration
        listModel.append({
                             "kind": "label",
                             "attributes": {
                                 "text": "Duration",
                                 "color": "" + Runtime.colors.accent.c100.text,
                                 "background": "" + Runtime.colors.accent.c100.background,
                                 "horizontalAlignment": Text.AlignRight,
                                 "font": {
                                     "pointSize": Runtime.minimumFontMetrics.font.pointSize,
                                     "weight": Font.Normal,
                                     "italic": false
                                 }
                             }
                         })
        plans.forEach( (plan) => {
                          listModel.append({
                                               "kind": "label",
                                               "attributes": {
                                                   "text": Utils.daysSpanAsString(plan.duration),
                                                   "color": "" + Runtime.colors.primary.c200.text,
                                                   "background": "" + Runtime.colors.primary.c200.background,
                                                   "horizontalAlignment": Text.AlignHCenter,
                                                   "font": {
                                                       "pointSize": Runtime.idealFontMetrics.font.pointSize,
                                                       "weight": Font.Normal,
                                                       "italic": false
                                                   }
                                               }
                                           })
                      })

        // Row: Device count
        listModel.append({
                             "kind": "labelWithTooltip",
                             "attributes": {
                                 "text": "Device Count",
                                 "tooltip": "Number of devices on which you can use the subscriptipon",
                                 "color": "" + Runtime.colors.accent.c100.text,
                                 "background": "" + Runtime.colors.accent.c100.background,
                                 "horizontalAlignment": Text.AlignRight,
                                 "font": {
                                     "pointSize": Runtime.minimumFontMetrics.font.pointSize,
                                     "weight": Font.Normal,
                                     "italic": false
                                 }
                             }
                         })
        plans.forEach( (plan) => {
                          listModel.append({
                                               "kind": "label",
                                               "attributes": {
                                                   "text": plan.devices,
                                                   "color": "" + Runtime.colors.primary.c100.text,
                                                   "background": "" + Runtime.colors.primary.c100.background,
                                                   "horizontalAlignment": Text.AlignHCenter,
                                                   "font": {
                                                       "pointSize": Runtime.idealFontMetrics.font.pointSize,
                                                       "weight": Font.Normal,
                                                       "italic": false
                                                   }
                                               }
                                           })
                      })

        // Rows for each feature
        let featureIndex = 0
        root.taxonomy.features.forEach( (feature) => {
                                           if(feature.group !== undefined && feature.group === true)
                                           return

                                           if(feature.display !== undefined && feature.display === false)
                                           return

                                           listModel.append({
                                                                "kind": "labelWithTooltip",
                                                                "attributes": {
                                                                    "text": feature.title,
                                                                    "tooltip": feature.description,
                                                                    "color": "" + Runtime.colors.accent.c100.text,
                                                                    "background": "" + Runtime.colors.accent.c100.background,
                                                                    "horizontalAlignment": Text.AlignRight,
                                                                    "font": {
                                                                        "pointSize": Runtime.minimumFontMetrics.font.pointSize,
                                                                        "weight": Font.Normal,
                                                                        "italic": false
                                                                    }
                                                                }
                                                            })

                                           for(let i=0; i<plans.length; i++) {
                                               const plan = plans[i]
                                               const featureEnabled = Scrite.isFeatureNameEnabled(feature.name, plan.features)
                                               const text = featureEnabled ? "✓" : "✗"
                                               const textColor = featureEnabled ? Runtime.colors.primary.c700.background : Runtime.colors.accent.a700.background
                                               const bgColor = (featureIndex%2 ? Runtime.colors.primary.c50.background : Runtime.colors.primary.c200.background)
                                               listModel.append({
                                                                    "kind": "label",
                                                                    "attributes": {
                                                                        "text": text,
                                                                        "color": "" + textColor,
                                                                        "background": "" + bgColor,
                                                                        "horizontalAlignment": Text.AlignHCenter,
                                                                        "font": {
                                                                            "pointSize": Runtime.minimumFontMetrics.font.pointSize + (featureEnabled ? 0 : 4),
                                                                            "weight": Font.Normal,
                                                                            "italic": false
                                                                        }
                                                                    }
                                                                })
                                           }

                                           featureIndex = featureIndex+1
                                       })

    }

    function populateFeatureListTableModel(subscription, listModel) {        
        let featureIndex = 0
        root.taxonomy.features.forEach( (feature) => {
                                           if(feature.group !== undefined && feature.group === true)
                                           return

                                           if(feature.display !== undefined && feature.display === false)
                                           return

                                           listModel.append({
                                                                "kind": "labelWithTooltip",
                                                                "attributes": {
                                                                    "text": feature.title,
                                                                    "tooltip": feature.description,
                                                                    "color": "" + Runtime.colors.accent.c100.text,
                                                                    "background": "" + Runtime.colors.accent.c100.background,
                                                                    "horizontalAlignment": Text.AlignRight,
                                                                    "font": {
                                                                        "pointSize": Runtime.minimumFontMetrics.font.pointSize,
                                                                        "weight": Font.Normal,
                                                                        "italic": false
                                                                    }
                                                                }
                                                            })

                                           const plan = subscription.plan
                                           const featureEnabled = Scrite.isFeatureNameEnabled(feature.name, plan.features)
                                           const text = featureEnabled ? "✓" : "✗"
                                           const textColor = featureEnabled ? Runtime.colors.primary.c700.background : Runtime.colors.accent.a700.background
                                           const bgColor = (featureIndex%2 ? Runtime.colors.primary.c50.background : Runtime.colors.primary.c200.background)
                                           listModel.append({
                                                                "kind": "label",
                                                                "attributes": {
                                                                    "text": text,
                                                                    "color": "" + textColor,
                                                                    "background": "" + bgColor,
                                                                    "horizontalAlignment": Text.AlignHCenter,
                                                                    "font": {
                                                                        "pointSize": Runtime.minimumFontMetrics.font.pointSize + (featureEnabled ? 0 : 4),
                                                                        "weight": Font.Normal,
                                                                        "italic": false
                                                                    }
                                                                }
                                                            })

                                           featureIndex = featureIndex+1
                                       })
    }

    QtObject {
        id: _private

        property Component planActivationApi: SubscriptionPlanActivationRestApiCall {
            property VclDialog waitDialog
            onJustIssuedCall: waitDialog = WaitDialog.launch("Activating plan ...")
            onFinished: {
                waitDialog.close()
                if(hasError) {
                    MessageBox.information("Error", errorMessage)
                } else {
                    UserAccountDialog.launch("Subscriptions")
                }
            }
        }

        function subscribeTo(plan, callList) {
            if(plan.action.kind === "get") {
                let api = planActivationApi.createObject(_private)
                api.activationApi = plan.action.api
                api.finished.connect(api.destroy)
                if( !api.call() ) {
                    api.destroy()
                    MessageBox.information("Error", "Unable to activate this plan. Please try again later.")
                } else {
                    if(callList)
                        callList.addCall(api)
                }
            } else if(plan.action.kind === "buy" ) {
                Qt.openUrlExternally(plan.action.url)
            }
        }

        function loadTaxonomy() {
            let api = Qt.createQmlObject("import io.scrite.components 1.0; AppPlanTaxonomyRestApiCall {}", _private)
            api.finished.connect( () => {
                                     if(api.hasError || !api.hasResponse)
                                        Utils.execLater(_private, 500, loadTaxonomy)
                                     else
                                        root.taxonomy = api.taxonomy
                                     api.destroy()
                                 })
            if(!api.call()) {
                api.destroy()
                Utils.execLater(_private, 500, loadTaxonomy)
            }
        }

        Component.onCompleted: loadTaxonomy()
    }
}
