// Biohazard "Z" badge in the vanilla status icon row while the local player is infected, tinted by stage.
modded class IngameHud
{
	protected ImageWidget m_ZV_Badge;
	protected int m_ZV_ShownStage = -1;

	override void Init(Widget hud_panel_widget)
	{
		super.Init(hud_panel_widget);
		if (!m_Badges)
			return;
		m_ZV_Badge = ImageWidget.Cast(g_Game.GetWorkspace().CreateWidgets("ZombieVirus/gui/layouts/zv_badge.layout", m_Badges));
		if (m_ZV_Badge)
		{
			m_ZV_Badge.LoadImageFile(0, PlayerBase.ZV_ICON);
			m_ZV_Badge.Show(false);
		}
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);
		ZV_UpdateBadge();
	}

	protected void ZV_UpdateBadge()
	{
		if (!m_ZV_Badge)
			return;
		int stage = -1;
		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (player && player.IsAlive())
			stage = player.ZV_GetHudStage();
		if (stage == m_ZV_ShownStage)
			return;
		m_ZV_ShownStage = stage;
		if (stage < 0)
		{
			m_ZV_Badge.Show(false);
		}
		else
		{
			m_ZV_Badge.SetColor(ZV_StageColor(stage));
			m_ZV_Badge.Show(true);
		}
		m_Badges.Update();
		if (stage >= 0)
			g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(ZV_LogBadge, 500, false);
	}

	// Diagnostics for the client log: where the badge ended up, next to a vanilla badge for comparison.
	protected void ZV_LogBadge()
	{
		if (!m_ZV_Badge)
			return;
		float x, y, w, h;
		m_ZV_Badge.GetScreenPos(x, y);
		m_ZV_Badge.GetScreenSize(w, h);
		string line = "[ZV] badge at " + x + "," + y + " size " + w + "x" + h + " visible " + m_ZV_Badge.IsVisibleHierarchy();
		Widget sick = m_Badges.FindAnyWidget("Sick");
		if (sick)
		{
			sick.GetScreenSize(w, h);
			line = line + " | vanilla Sick size " + w + "x" + h;
		}
		Print(line);
	}

	protected int ZV_StageColor(int stage)
	{
		switch (stage)
		{
			case 1:
				return ARGB(255, 230, 200, 60);
			case 2:
				return ARGB(255, 235, 130, 40);
			case 3:
				return ARGB(255, 220, 50, 40);
		}
		return ARGB(255, 220, 220, 220);	// stage 0, same grey as the vanilla icons
	}
}
