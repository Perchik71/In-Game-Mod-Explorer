ScriptName espE_LaunchQuestPlayerAliasScript extends ReferenceAlias
; ---------------------------------------------------------------------------------

GlobalVariable Property  espEg_LaunchFlag 	Auto Mandatory

; ------------------------
Quest Property espE_MainQuest 	auto Mandatory

Event OnAliasInit()
	; Debug.Trace("espE_LaunchQuestPlayerAliasScript OnAliasInit")
	Init()    
EndEvent

Event OnPlayerLoadGame()
	; Debug.Trace("espE_LaunchQuestPlayerAliasScript OnPlayerLoadGame")
    Init()
EndEvent

Function Init()
	; GoTostate("")
	ScriptObject FuncScript = espE_MainQuest.CastAs("espE_MainQuestScript") as ScriptObject
	FuncScript.CallFunctionNoWait("ResetMain", none)
	; Debug.Trace("espE_LaunchQuestPlayerAliasScript Init ResetMain1")

	; GoTostate("WaitingFlagState")

	; FuncScript = self as ScriptObject
	; FuncScript.CallFunctionNoWait("WaitingFlag", none)
EndFunction

; State WaitingFlagState
; 	Function WaitingFlag()
; 		While (GetState() == "WaitingFlagState")
; 			While ((GetState() == "WaitingFlagState") && (espEg_LaunchFlag.Value as int) != 1)
; 				Utility.Wait(0.1)						
; 			EndWhile				

; 			if GetState() == "WaitingFlagState"
; 				ScriptObject QuestScript = espE_MainQuest.CastAs("espE_MainQuestScript") as ScriptObject
; 				Debug.Trace("espE_LaunchQuestPlayerAliasScript WaitingFlag StartExplorer")

; 				QuestScript.CallFunctionNoWait("StartExplorer", none)
; 				Utility.Wait(3)
; 			endif
; 		EndWhile		
; 	EndFunction
; EndState

; Function WaitingFlag()
		
; EndFunction
