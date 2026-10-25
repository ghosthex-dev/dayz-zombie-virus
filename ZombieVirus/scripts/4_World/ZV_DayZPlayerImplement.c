modded class ActionManagerBase
{
	override bool ActionPossibilityCheck(int pCurrentCommandID)
	{
		if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_SCRIPT)
			return true;
		return super.ActionPossibilityCheck(pCurrentCommandID);
	}
}

class HumanCommandScript_ZVInfectedPlrST
{
	int m_CmdStartControlInfected;

	void HumanCommandScript_ZVInfectedPlrST(Human pHuman)
	{
		HumanAnimInterface hai = pHuman.GetAnimInterface();
		m_CmdStartControlInfected = hai.BindCommand("CMD_Vehicle_GetIn");
	}
}

class HumanCommandScript_ZVInfectedControl : HumanCommandScript
{
	protected DayZPlayer m_pPlayer;
	protected HumanCommandScript_ZVInfectedPlrST m_Table;

	void HumanCommandScript_ZVInfectedControl(DayZPlayer pPlayer, HumanCommandScript_ZVInfectedPlrST pTable)
	{
		m_pPlayer = pPlayer;
		m_Table = pTable;
	}

	override void OnActivate()
	{
		dBodyEnableGravity(m_pPlayer, false);
		PreAnim_CallCommand(m_Table.m_CmdStartControlInfected, 0, 0);
	}

	override void OnDeactivate()
	{
		dBodyEnableGravity(m_pPlayer, true);
	}

	override bool PostPhysUpdate(float pDt)
	{
		return true;
	}
}

modded class DayZPlayerImplement
{
	protected ref HumanCommandScript_ZVInfectedPlrST m_ZV_InfectedPlrST;

	void ZV_StartCommand_InfectedControl()
	{
		if (!m_ZV_InfectedPlrST)
			m_ZV_InfectedPlrST = new HumanCommandScript_ZVInfectedPlrST(this);

		StartCommand_Script(new HumanCommandScript_ZVInfectedControl(this, m_ZV_InfectedPlrST));
	}

	override bool ModCommandHandlerAfter(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
		if (super.ModCommandHandlerAfter(pDt, pCurrentCommandID, pCurrentCommandFinished))
			return true;

		if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_SCRIPT)
		{
			HumanCommandScript hcs = GetCommand_Script();
			if (HumanCommandScript_ZVInfectedControl.Cast(hcs))
				return true;
			return false;
		}

		if (GetParent() && ZombieBase.Cast(GetParent()))
		{
			ZV_StartCommand_InfectedControl();
			return true;
		}

		return false;
	}
}
