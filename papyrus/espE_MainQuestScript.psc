ScriptName espE_MainQuestScript extends Quest 
; ---------------------------------------------------------------------------------
Group GlobalVariables
	GlobalVariable Property  espEg_Ammo 		Auto Mandatory
	GlobalVariable Property  espEg_Armor 		Auto Mandatory
	GlobalVariable Property  espEg_Book 		Auto Mandatory
	GlobalVariable Property  espEg_Flora 		Auto Mandatory
	GlobalVariable Property  espEg_Key 			Auto Mandatory
	GlobalVariable Property  espEg_Note 		Auto Mandatory
	GlobalVariable Property  espEg_Weapon 		Auto Mandatory
	GlobalVariable Property  espEg_MiscItem		Auto Mandatory
	GlobalVariable Property  espEg_WeaponMod	Auto Mandatory
	GlobalVariable Property  espEg_Alchemy		Auto Mandatory

	GlobalVariable Property  espEg_ItemQuantity Auto Mandatory
	GlobalVariable Property  espEg_Loop 		Auto Mandatory
	GlobalVariable Property  espEg_Container 	Auto Mandatory
	GlobalVariable Property  espEg_LaunchFlag 	Auto Mandatory

	GlobalVariable Property  espEg_PartlyRedItemQuantity 	Auto Mandatory
	GlobalVariable Property  espEg_PartlyReadThreshold 		Auto Mandatory

EndGroup
; ------------------------
Group FormLists
	FormList 		Property espEListCom		Auto Mandatory
	FormList 		Property espEDisabledItem	Auto Mandatory
	FormList 		Property espEExpand			Auto Mandatory

	FormList 		Property espE_ListAlchemy 	Auto Mandatory
	FormList 		Property espE_ListAmmo 		Auto Mandatory
	FormList 		Property espE_ListWeapon 	Auto Mandatory
	FormList 		Property espE_ListArmor 	Auto Mandatory
	FormList 		Property espE_ListBook 		Auto Mandatory
	FormList 		Property espE_ListNote 		Auto Mandatory
	FormList 		Property espE_ListMod 		Auto Mandatory
	FormList 		Property espE_ListKey 		Auto Mandatory
	FormList 		Property espE_ListMiscItem	Auto Mandatory
EndGroup
; ------------------------
Group ExpandItem
	Potion 			Property espE_ExpandAlchemy 		Auto Mandatory
	Ammo 	 		Property espE_ExpandAmmo 			Auto Mandatory
	Weapon 	 		Property espE_ExpandWeapon 			Auto Mandatory
	Armor 	 		Property espE_ExpandArmor 			Auto Mandatory
	Book 	 		Property espE_ExpandBook 			Auto Mandatory
	Holotape 		Property espE_ExpandNote 			Auto Mandatory
	Key 			Property espE_ExpandKey 			Auto Mandatory
	MiscObject 		Property espE_ExpandMiscItem		Auto Mandatory
	MiscObject 		Property espE_ExpandMod 			Auto Mandatory
EndGroup
; ------------------------
Group Misc
	Message 		Property espEmsgTrigger 			Auto Mandatory

	Message 		Property espE_MsgBusyExpand 		Auto Mandatory const
	ObjectReference Property REHoldingCellMarker 		Auto Mandatory const
	Container 		Property espE_Containter 	 		Auto Mandatory const
EndGroup

; ---------------------------------------------------------------------------------

Struct ItemListStru
	FormList List 
	Form     Item 
	int  	 Num 
	bool 	 StopFlag
EndStruct 
; ---------------------------------------------------------------------------------
bool bContainerClosed = false
int  NUM_LIMIT = 30
ItemListStru[] ItemList
ObjectReference espContainer
int  	PartlyReadNum
int 	itemQuan
; ------------------------
Function CleanContainer()
	if espContainer
		Form[] AllItemsInContainer = espContainer.GetInventoryItems()
		int i = AllItemsInContainer.Length
		While i
			i -= 1
			Form ItemInContainer = AllItemsInContainer[i]
			int ItemCount = espContainer.GetItemCount(ItemInContainer)
			While ItemCount > 0
				If (ItemCount <= 65535)
					espContainer.RemoveItem(ItemInContainer, ItemCount, true)
					ItemCount = 0
				Else
					espContainer.RemoveItem(ItemInContainer, 65535, true)
					ItemCount -= 65535
				EndIf
			EndWhile 
		EndWhile 
		espContainer.RemoveAllItems() 
		; Debug.Notification("clean")
	endif
EndFunction
; ------------------------
Function StartExplorer()
	GotoState("ExploreStatus")

	espEg_LaunchFlag.Value = 100

	bool bLoopExplore = true

	ItemList = new ItemListStru[0]
	ItemListStru ItemNew 

	ItemNew = new ItemListStru
	ItemNew.List = espE_ListAlchemy 
	ItemNew.Item = espE_ExpandAlchemy as Form
	ItemList.Add(ItemNew)

	ItemNew = new ItemListStru
	ItemNew.List = espE_ListAmmo 		
	ItemNew.Item = espE_ExpandAmmo as Form
	ItemList.Add(ItemNew)


	ItemNew = new ItemListStru
	ItemNew.List = espE_ListWeapon 
	ItemNew.Item = espE_ExpandWeapon as Form
	ItemList.Add(ItemNew)

	ItemNew = new ItemListStru
	ItemNew.List = espE_ListArmor 	
	ItemNew.Item = espE_ExpandArmor as Form
	ItemList.Add(ItemNew)

	ItemNew = new ItemListStru
	ItemNew.List = espE_ListBook 		
	ItemNew.Item = espE_ExpandBook as Form
	ItemList.Add(ItemNew)

	ItemNew = new ItemListStru
	ItemNew.List = espE_ListNote 		
	ItemNew.Item = espE_ExpandNote as Form
	ItemList.Add(ItemNew)

	ItemNew = new ItemListStru
	ItemNew.List = espE_ListMod 		
	ItemNew.Item = espE_ExpandMod as Form
	ItemList.Add(ItemNew)

	ItemNew = new ItemListStru
	ItemNew.List = espE_ListKey 		
	ItemNew.Item = espE_ExpandKey as Form
	ItemList.Add(ItemNew)

	ItemNew = new ItemListStru
	ItemNew.List = espE_ListMiscItem
	ItemNew.Item = espE_ExpandMiscItem as Form
	ItemList.Add(ItemNew)


	While (bLoopExplore)
		int i = 0
		; Debug.Trace("ItemList.length" + ItemList.Length)
		While (i < ItemList.Length)
			ItemList[i].Num = 0
			ItemList[i].StopFlag = 0
			ItemList[i].List.Revert()
			i += 1
		EndWhile

		bLoopExplore = false
		; Debug.Trace("espEListCom.GetSize():" + espEListCom.GetSize())
		int result = espEmsgTrigger.show()
		; int result = 100
		; Debug.Notification("result:" + result)
		if result == 100 
			if espContainer
				CleanContainer()
			else
				espContainer = REHoldingCellMarker.PlaceAtMe(espE_Containter)
				; espContainer = REHoldingCellMarker.PlaceAtMe(game.getform(0x4b6e9)) 
			endif

			; Debug.Trace("After espEListCom.GetSize():" + espEListCom.GetSize())
			itemQuan = espEg_ItemQuantity.Value as int;
			
			if itemQuan < 1 
				itemQuan = 5
			elseif itemQuan > 10
				itemQuan = 10
			endif

			i = 0
			int totalNum = 0
			While (i < ItemList.Length)
				totalNum += ItemList[i].List.GetSize()
				;Debug.Trace("i:" + i + " num:" + ItemList[i].List.GetSize())
				i += 1
			EndWhile

			bool bPartlyRead = (totalNum > espEg_PartlyReadThreshold.Value)
			if bPartlyRead
				PartlyReadNum = espEg_PartlyRedItemQuantity.Value as int
			else
				PartlyReadNum = 999999
			endif

			i = 0
			while (i < ItemList.Length )
				int _num = ItemList[i].List.GetSize()
				; int itemAdded = itemQuan
				; if ItemList[i].List == espE_ListKey
				; 	itemAdded = 1
				; elseif ItemList[i].List == espE_ListAmmo
				; 	itemAdded = 500 * itemAdded
				; endif
				; if _num != 0
				; 	Debug.Trace("itemAdded:" + itemAdded + " " + ItemList[i].List)
				; 	espContainer.AddItem(ItemList[i].List, itemAdded, true)
				; endif
				int j = 0
				While (j < _num && j < PartlyReadNum)
					Form item = ItemList[i].List.GetAt(j)
					; int itemAdded = itemQuan
					; if ItemList[i].List == espE_ListKey
					; 	itemAdded = 1
					; elseif ItemList[i].List == espE_ListAmmo
					; 	itemAdded = 500 * itemAdded
					; endif
					int itemAdded = GetItemNum(item)
					espContainer.AddItem(item, itemAdded, true)

					j += 1
					if j >= PartlyReadNum
						;;Debug.Trace("itemLimit i:" + i + " num:" + _num + ItemList[i].Item)
						espContainer.AddItem(ItemList[i].Item, 1, true)
					endif	
				EndWhile

				if (j >= _num)
					ItemList[i].List.Revert()
				endif
					
				i += 1
			EndWhile

			; int money = espContainer.GetItemCount(Game.GetForm(0x0F)) * 10000
			; espContainer.AddItem(Game.GetForm(0x0F), money, true) 

			;Debug.Trace("End Add items")
			; AddInventoryEventFilter(espEExpand)
			AddInventoryEventFilter(none)
			RegisterForRemoteEvent(Game.GetPlayer(), "OnItemAdded")
			RegisterForRemoteEvent(Game.GetPlayer(), "OnPlayerLoadGame")

			RegisterForMenuOpenCloseEvent("ContainerMenu")
			Utility.Wait(0.1)

			bContainerClosed = false
			espContainer.Activate(Game.GetPlayer())

			While (!bContainerClosed && GetState() != "")
				Utility.Wait(0.2)
			EndWhile

			bLoopExplore = (espEg_Loop.Value != 0.0) && (GetState() != "")
		EndIf	
	EndWhile

	ResetMain()
	; Debug.Notification("end end end end here")
	GotoState("")
EndFunction
; ------------------------
bool bExpandItemLock = false
Function ExpandItem(Form akBaseItem)
	;Debug.Notification("start")
	bExpandItemLock = true

	int i = 0
	ItemListStru _itemList = none
	While (i < ItemList.length)
		if akBaseItem == ItemList[i].Item
			Debug.Trace("expanditem i:" + i)
			_itemList = ItemList[i]
			i = 999999
		endif
		i += 1		
	EndWhile

	if !_itemList
		return 
	endif

	i = PartlyReadNum 	
	int _num = _itemList.List.GetSize()
	
	; int itemAdded = itemQuan
	; if _itemList.List == espE_ListKey
	; 	itemAdded = 1
	; elseif _itemList.List == espE_ListAmmo
	; 	itemAdded = 500 * itemAdded
	; endif

	; ObjectReference tmp = REHoldingCellMarker.PlaceAtMe(espE_Containter) 
	ObjectReference tmp = espContainer
	While (i < _num && GetState() != "" && !bContainerClosed && espContainer && bExpandItemLock)
		Form item = _itemList.List.GetAt(i)
		int itemAdded = GetItemNum(item)
		tmp.AddItem(item, itemAdded, true)
		;Debug.Trace("i:" + i + " " + _itemList.List.GetAt(i))
		i += 1
	EndWhile

	; int money = tmp.GetItemCount(Game.GetForm(0x0F)) * 10000
	; tmp.AddItem(Game.GetForm(0x0F), money, true) 
	;Debug.Notification("end")
	;tmp.RemoveAllItems(espContainer)
	bExpandItemLock = false
EndFunction
; ---------------------------------------------------------------------------------
int Function GetItemNum(Form akBaseItem)
	int itemAdded = espEg_ItemQuantity.Value as int;

	if itemAdded < 1 
		itemAdded = 5
	elseif itemAdded > 10
		itemAdded = 10
	endif	

	if akBaseItem is key
		itemAdded = 1
	elseif akBaseItem is ammo
		itemAdded = 500 * itemAdded
	elseif akBaseItem.GetFormID() == 0X0F ; Caps
		itemAdded = itemAdded * 10000
	endif

	return itemAdded
EndFunction
; ---------------------------------------------------------------------------------
Event Actor.OnPlayerLoadGame(Actor akSender)
	bContainerClosed = true    
	GotoState("")
EndEvent
; ---------------------------------------------------------------------------------
Event ObjectReference.OnItemAdded(ObjectReference akSender, Form akBaseItem, int aiItemCount, ObjectReference akItemReference, ObjectReference akSourceContainer)
	;;Debug.Notification("ItemAdded " + akBaseItem)
	if espEExpand.Find(akBaseItem) >= 0
		Game.GetPlayer().RemoveItem(akBaseItem, 9999, true)
		if bExpandItemLock
			if espContainer
				espE_MsgBusyExpand.Show()
				espContainer.AddItem(akBaseItem, 1, true)
			endif
		else
			var[] Paras = new Var[1]
			paras[0] = akBaseItem
			(self as ScriptObject).CallFunctionNoWait("ExpandItem", Paras)
		endif
	elseif espContainer && espContainer.GetItemCount(akBaseItem) == 0
		Debug.Trace("OnItemAdd:" + akBaseItem)
		int itemAdded = GetItemNum(akBaseItem)
		if itemAdded != 1 
			espContainer.AddItem(akBaseItem, itemAdded * 100, true)
		endif
	endif
EndEvent
; ---------------------------------------------------------------------------------
Event OnMenuOpenCloseEvent(string asMenuName, bool abOpening)
    if asMenuName == "ContainerMenu" && !abOpening
    	bContainerClosed = true
    	UnregisterForMenuOpenCloseEvent("ContainerMenu")
    endif
EndEvent
; ---------------------------------------------------------------------------------
State ExploreStatus
	Function StartExplorer()
	EndFunction
EndState

; ---------------------------------------------------------------------------------
int hotKey 
int flagAZERTY
bool shiftPress = false
Event OnKeyUp(int keyCode, float time)
	; Debug.MessageBox("key press:" + keyCode)
	Debug.Trace("==========================keyCode:" + keyCode + " shift:" + espE_f4seFuncs.GetShiftPress())
	if (keyCode == hotKey)
;	|| (keyCode == 0x5a && espE_f4seFuncs.GetShiftPress() && !flagAZERTY) || (keyCode == 0x57 && espE_f4seFuncs.GetShiftPress() && flagAZERTY)
		StartExplorer()
	elseif keyCode == 0x10
		shiftPress = false
	endif

EndEvent

; Event OnKeyDown(int keyCode)
; 	; Debug.MessageBox("key press:" + keyCode)
; 	Debug.Trace("==========================keyCode down:" + keyCode)
; 	if keyCode == 0x10
; 		; shift key
; 		shiftPress = true
; 	endif
; EndEvent

; ---------------------------------------------------------------------------------
Function test()
	Debug.Trace("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
	Debug.Trace("espE_MainQuestScript OnPlayerLoadGame On_espEActive")
	Debug.Trace("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
	; StartExplorer()
EndFunction
; ---------------------------------------------------------------------------------
Function SetNewHotkey()
	UnregisterForKey(hotKey)
	hotKey = espe_f4sefuncs.GetINIInt("General", "hotKey", 121)
    ; flagAZERTY = espe_f4sefuncs.GetINIInt("General", "AZERTY", 0) ; #DEBUG_LINE_NO:391
	espe_f4sefuncs.ResetHotKey(hotKey)
	RegisterForKey(hotKey)
EndFunction
; ---------------------------------------------------------------------------------
Function ResetMain()
	Debug.Trace("espE_MainQuestScript Reset")
	hotKey = espE_f4seFuncs.GetINIInt("General", "HotKey", 121)
	; flagAZERTY = espE_f4seFuncs.GetINIInt("General", "AZERTY", 0)

	; Debug.MessageBox("value:" + hotKey)
	GotoState("")
	; UnregisterForExternalEvent("On_espEActive")
	UnregisterForKey(hotKey)
	; UnregisterForKey(0x5a)
	; UnregisterForKey(0x57)
	; UnregisterForKey(0x10)


	espE_ListAlchemy.Revert() 
	espE_ListAmmo.Revert() 		
	espE_ListWeapon.Revert() 	
	espE_ListArmor.Revert() 	
	espE_ListBook.Revert() 		
	espE_ListNote.Revert() 		
	espE_ListMod.Revert() 		
	espE_ListKey.Revert() 		
	espE_ListMiscItem.Revert()

	espEListCom.Revert()

	RemoveAllInventoryEventFilters()
	UnregisterForAllRemoteEvents()
	UnregisterForMenuOpenCloseEvent("ContainerMenu")

	if espContainer
		CleanContainer()
	endif
	espContainer = none

	espEg_LaunchFlag.Value = 0
	; Debug.Notification("end end end end here")
	bExpandItemLock = false

	; Debug.Trace("register external event")
	; RegisterForExternalEvent("On_espEActive", "test")
	; RegisterForExternalEvent("On_espEActive", "StartExplorer")
	RegisterForKey(hotKey)
	; RegisterForKey(0x10)
	; RegisterForKey(0x5a)
	; RegisterForKey(0x57)

EndFunction