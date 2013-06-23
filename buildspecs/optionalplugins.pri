# To build the optional plugins you can use two methods:
# 1) add to the qmake commande line:
#    CONFIG+=with-plugin
#    where 'plugin' is the config name of the plugin
# 2) Use this file
#    Uncomment required plugins


# ***************************************************************************************
# *                                AccountNew Plugin                                    *
# ***************************************************************************************
# Uncomment the following line to build the plugin
# CONFIG *= with-account
# ***************************************************************************************


# ***************************************************************************************
# *                                AccountOld Plugin (obsolete)                         *
# ***************************************************************************************
# Uncomment the following line to build the plugin
# CONFIG *= with-account-old
# ***************************************************************************************


# ***************************************************************************************
# *                                  Agenda Plugin                                      *
# ***************************************************************************************
# Uncomment the following line to build the plugin
 CONFIG *= with-agenda
# ***************************************************************************************


# ***************************************************************************************
# *                                  Alert Plugin                                       *
# ***************************************************************************************
# Uncomment the following line to build the plugin
 CONFIG *= with-alerts
# ***************************************************************************************


# ***************************************************************************************
# *                                 Feedback Plugin                                     *
# ***************************************************************************************
# Uncomment the following line to build the plugin
 CONFIG *= with-feedback
# ***************************************************************************************


# ***************************************************************************************
# *                                 PadTools Plugin                                     *
# ***************************************************************************************
# Uncomment the following line to build the plugin
 CONFIG *= with-pad
# ***************************************************************************************


# ***************************************************************************************
# *                                   PMHx Plugin                                       *
# ***************************************************************************************
# Uncomment the following line to build the plugin
 CONFIG *= with-pmh
# ***************************************************************************************


# ***************************************************************************************
# *                                  Webcam Plugin                                      *
# ***************************************************************************************
# Uncomment the following line to build the plugin
 CONFIG *= with-webcam
# ***************************************************************************************


# The following lines must not be modified
with-agenda   { DEFINES *= WITH_AGENDA }
with-alerts   { DEFINES *= WITH_ALERTS }
with-feedback { DEFINES *= WITH_FEEDBACK }
with-pad      { DEFINES *= WITH_PAD }
with-pmh      { DEFINES *= WITH_PMH }
with-webcam   { DEFINES *= WITH_WEBCAM }
