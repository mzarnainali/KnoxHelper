const char P_DASH_BOARD[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Knox Dashboard</title>
<link rel='icon' href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAAAXNSR0IArs4c6QAAAl9JREFUWEe9lzFoFEEUhr+X3B4KFtFKC0kVLESD2CmilgopbCxUUBRUvJtVS0FBRCGVgd27qCAqKFgIFiJoGSGliEQrTSEWAUGx0MLc3N2TC8lxnre7M7vGbfef/33zz2PnrZDxBLHeAU5n6ZbeK/dsKKectMsiSRVP6UhQ4ruH4RdrZKOHnlSAINIdCG88DN9ZI9s99OkA5ZoeUuWpq6HCpaaRSVd9R5eeQKwXgZuuhkNDjC1WZN5V7wIQAcbJUJm1oexx0vaIUhMo1fSZKBOOpqE1Ejtqu7KsI5gDtrmY5onf5Qh+AOuyAAReNIwczNINep+YwNq6bm62+exomiv+1ARKke4VYcYRIFGmsCAwmdQfiQkEkZ5AuF8UYHn9jDWy3+sIyrFeU7jyjwCmrZGKF0AQ60PgWGEA4etQi12L5+WjL8AssLsogMLlppEbST6JPVCOdUFhUyEAYc5WZTzNI7kJY+1cwyOFAOCkNZLayGkA7oPIAEoRXjaqciBrA+kDScrqYFp30uJ1kkSVfc1QXq0eQF3P0Ob2wAIeo1n+BGK9Cwya/37ab6znqjSzdp95GaUZBLG+Bf7qcFGuN0Jx/oDlSiCIdByhA9D/fLBGtrjsfEWTD6CmFZRaf6G2MNGqyvPVB4j1AXC8r1DihZPrQ5Rx/u+Brb0aO8wGzonPP8TScu8jWFPT0Zby6Q9ApW5DqfpEn7sHypEeVeFRT7GmNRLkKZ4rgSDWW8DZ7g6UI41QHv9PgN5Jed4aGctbPG8C3VtyuMXorwviOrgO5PRuwpXfdYHDDSNPiuy+s/Y3kdm/IT2Yw5EAAAAASUVORK5CYII=' type='image/x-png' />

<link href="https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@400;500;600&family=Orbitron:wght@400;700&display=swap" rel="stylesheet">
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }

  body {
    font-family: 'Space Grotesk', sans-serif;
    background: #0a0f1a;
    color: #e8edf5;
    min-height: 100vh;
  }

  .solar-dash {
    margin: 0 auto;
    position: relative;
    overflow: hidden;
	width: 80%;
  }

  .solar-dash::before {
    content: '';
    position: fixed;
    top: -120px; right: -80px;
    width: 400px; height: 400px;
    background: radial-gradient(circle, rgba(186,117,23,0.12) 0%, transparent 70%);
    pointer-events: none;
    z-index: 0;
  }

  .solar-dash::after {
    content: '';
    position: fixed;
    bottom: -80px; left: -60px;
    width: 300px; height: 300px;
    background: radial-gradient(circle, rgba(29,158,117,0.08) 0%, transparent 70%);
    pointer-events: none;
    z-index: 0;
  }

  /* ── HEADER ── */
  .header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 18px 20px 14px;
    border-bottom: 0.5px solid rgba(255,255,255,0.07);
    position: relative;
    z-index: 1;
  }

  .logo {
    font-family: 'Orbitron', monospace;
    font-size: 13px;
    font-weight: 700;
    letter-spacing: 2px;
    color: #FAC775;
    display: flex;
    align-items: center;
    gap: 8px;
  }

  .logo-icon {
    width: 28px; height: 28px;
    border-radius: 6px;
    display: flex; align-items: center; justify-content: center;
    font-size: 20px;
    color: #2196F3;
  }

  .header-right { display: flex; align-items: center; gap: 10px; }

  .time-display {
    font-size: 11px;
    color: rgba(232,237,245,0.4);
    font-variant-numeric: tabular-nums;
  }

  .status-pill {
    display: flex; align-items: center; gap: 5px;
    padding: 4px 10px;
    border-radius: 20px;
    font-size: 10px; font-weight: 500; letter-spacing: 0.5px;
  }

  .status-pill.online  { background: rgba(29,158,117,0.15); color: #5DCAA5; border: 0.5px solid rgba(29,158,117,0.3); }
  .status-pill.offline { background: rgba(226,75,74,0.15);  color: #F09595; border: 0.5px solid rgba(226,75,74,0.3); }

  .dot { width: 6px; height: 6px; border-radius: 50%; }
  .dot.green { background: #5DCAA5; box-shadow: 0 0 6px #5DCAA5; animation: pulse-green 2s infinite; }
  .dot.red   { background: #F09595; }

  @keyframes pulse-green { 0%,100%{opacity:1} 50%{opacity:0.4} }

  /* ── GRID ── */
  .main-grid {
    display: grid;
    grid-template-columns: 1fr 1fr 1fr;
    gap: 10px;
    padding: 14px 16px;
    position: relative; z-index: 1;
  }

  .section-divider {
    font-size: 9px;
    color: rgba(232,237,245,0.2);
    letter-spacing: 1.5px;
    text-transform: uppercase;
    padding: 2px 0 0;
    grid-column: 1 / -1;
  }

  /* ── HERO CARDS ── */
  .hero-card {
    grid-column: 1 / -1;
    border-radius: 14px;
    padding: 16px 20px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    position: relative;
    overflow: hidden;
  }

  .hero-card.solar {
    background: linear-gradient(135deg, rgba(186,117,23,0.15) 0%, rgba(239,159,39,0.08) 100%);
    border: 0.5px solid rgba(250,199,117,0.2);
  }
  .hero-card.solar::before {
    content: '';
    position: absolute; right: -20px; top: -20px;
    width: 140px; height: 140px;
    background: radial-gradient(circle, rgba(250,199,117,0.12) 0%, transparent 70%);
  }

  .hero-card.ac {
    background: linear-gradient(135deg, rgba(29,158,117,0.15) 0%, rgba(93,202,165,0.07) 100%);
    border: 0.5px solid rgba(93,202,165,0.2);
  }
  .hero-card.ac::before {
    content: '';
    position: absolute; right: -20px; top: -20px;
    width: 140px; height: 140px;
    background: radial-gradient(circle, rgba(93,202,165,0.1) 0%, transparent 70%);
  }

  .hero-label {
    font-size: 10px; letter-spacing: 1.5px; text-transform: uppercase; margin-bottom: 6px;
  }
  .solar .hero-label { color: rgba(250,199,117,0.6); }
  .ac    .hero-label { color: rgba(93,202,165,0.6); }

  .hero-value {
    font-family: 'Orbitron', monospace;
    font-size: 36px; font-weight: 700; line-height: 1; margin-bottom: 10px;
  }
  .solar .hero-value { color: #FAC775; }
  .ac    .hero-value { color: #5DCAA5; }

  .hero-stats { display: flex; gap: 20px; }

  .hero-stat-val {
    font-family: 'Orbitron', monospace;
    font-size: 13px; font-weight: 700;
  }
  .solar .hero-stat-val { color: rgba(250,199,117,0.85); }
  .ac    .hero-stat-val { color: rgba(93,202,165,0.85); }

  .hero-stat-lbl {
    font-size: 9px; letter-spacing: 0.8px; text-transform: uppercase;
    color: rgba(232,237,245,0.3); margin-top: 1px;
  }

  /* ── ICON RINGS ── */
  .icon-ring {
    width: 72px; height: 72px; border-radius: 50%;
    display: flex; align-items: center; justify-content: center;
    position: relative; flex-shrink: 0;
  }
  .icon-ring.solar { border: 2px solid rgba(250,199,117,0.3); }
  .icon-ring.solar.off {    color: #1c1b1b; }
  .icon-ring.solar::before {
    content: ''; position: absolute; inset: 4px; border-radius: 50%;
    background: radial-gradient(circle, rgba(250,199,117,0.3), transparent);
    animation: sun-pulse 3s ease-in-out infinite;
  }

  .icon-ring.solar.off::before {
  content: none;        /* removes pseudo-element completely */
}
  .icon-ring.ac { border: 2px solid rgba(93,202,165,0.3); }
  .icon-ring.ac::before {
    content: ''; position: absolute; inset: 4px; border-radius: 50%;
    background: radial-gradient(circle, rgba(93,202,165,0.2), transparent);
    animation: ac-pulse 3s ease-in-out infinite;
  }
  .icon-inner { font-size: 28px; position: relative; z-index: 1; }

  @keyframes sun-pulse { 0%,100%{opacity:0.6;transform:scale(1)} 50%{opacity:1;transform:scale(1.1)} }
  @keyframes ac-pulse  { 0%,100%{opacity:0.5;transform:scale(1)} 50%{opacity:1;transform:scale(1.08)} }

  /* ── SYSTEM LOAD + TEMP ── */
  .combo-card {
    grid-column: 1 / -1;
    background: rgba(255,255,255,0.03);
    border: 0.5px solid rgba(255,255,255,0.07);
    border-radius: 12px;
    padding: 14px 18px;
    position: relative; overflow: hidden;
  }
  .combo-card::after {
    content: ''; position: absolute; bottom: 0; left: 0; right: 0; height: 2px;
    border-radius: 0 0 12px 12px;
    background: linear-gradient(90deg, transparent, rgba(29,158,117,0.5), transparent);
  }
  .combo-header { display: flex; align-items: center; gap: 8px; margin-bottom: 12px; }
  .combo-icon {
    width: 26px; height: 26px; border-radius: 7px;
    display: flex; align-items: center; justify-content: center;
    font-size: 13px; background: rgba(29,158,117,0.12);
  }
  .combo-title { font-size: 10px; letter-spacing: 1.2px; color: rgba(232,237,245,0.35); text-transform: uppercase; }

  .load-temp-grid { display: flex; gap: 16px; align-items: center; }
  .load-side { flex: 1; }
  .temp-side {
    min-width: 80px; text-align: center;
    border-left: 0.5px solid rgba(255,255,255,0.07);
    padding-left: 16px;
  }
  .temp-val {
    font-family: 'Orbitron', monospace;
    font-size: 22px; font-weight: 700; color: #F09595; margin-bottom: 2px;
  }
  .temp-lbl { font-size: 9px; color: rgba(232,237,245,0.35); letter-spacing: 0.8px; text-transform: uppercase; }
  .load-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px; }
  .load-pct-val { font-family: 'Orbitron', monospace; font-size: 18px; font-weight: 700; color: #5DCAA5; }
  .load-lbl { font-size: 9px; color: rgba(232,237,245,0.35); letter-spacing: 0.8px; text-transform: uppercase; }
  .load-bar-track { height: 6px; background: rgba(255,255,255,0.06); border-radius: 10px; overflow: hidden; }
  .load-bar-fill {
    height: 100%;
    background: linear-gradient(90deg, #1D9E75, #5DCAA5);
    border-radius: 10px;
    transition: width 0.8s cubic-bezier(0.4,0,0.2,1);
    width: 0%;
    box-shadow: 0 0 8px rgba(29,158,117,0.4);
  }

  /* ── ENERGY ROUTING ── */
  .flow-section {
    grid-column: 3;
	grid-row: 1 / span 2;
    background: rgba(255,255,255,0.03);
    border: 0.5px solid rgba(255,255,255,0.07);
    border-radius: 12px;
    padding: 16px 18px;
  }
  .flow-title {
    font-size: 10px; letter-spacing: 1.2px; color: rgba(232,237,245,0.35);
    text-transform: uppercase; margin-bottom: 16px;
  }
  .flow-diagram { display: flex; align-items: center; justify-content: center; gap: 0; }
  .flow-sources  { display: flex; flex-direction: column; gap: 12px; }

  .flow-node-box {
    border-radius: 12px; padding: 10px 14px;
    text-align: center; min-width: 90px;
  }
  .flow-node-box.pv   { background: rgba(250,199,117,0.10); border: 0.5px solid rgba(250,199,117,0.30); }
  .flow-node-box.grid { background: rgba(55,138,221,0.10);  border: 0.5px solid rgba(55,138,221,0.30); }
  .flow-node-box.load { background: rgba(29,158,117,0.12);  border: 0.5px solid rgba(29,158,117,0.35); }

  .flow-node-icon { font-size: 18px; margin-bottom: 3px; }
  .flow-node-name { font-size: 9px; letter-spacing: 0.8px; text-transform: uppercase; color: rgba(232,237,245,0.4); margin-bottom: 2px; }
  .flow-node-watts { font-family: 'Orbitron', monospace; font-size: 13px; font-weight: 700; }
  .flow-node-watts.pv-color   { color: #FAC775; }
  .flow-node-watts.grid-color { color: #85B7EB; }
  .flow-node-watts.load-color { color: #5DCAA5; }

  .flow-middle {
    display: flex; flex-direction: column;
    align-items: center; justify-content: center;
    gap: 50px; padding: 0 12px; flex: 1;
  }

  .flow-arrow-row { display: flex; align-items: center; gap: 0; width: 100%; }

  .flow-arrow-line {
    flex: 1; height: 4px;
    background: rgb(255 255 255 / 34%);
    border-radius: 10px;
    position: relative; overflow: hidden;
  }
  .flow-arrow-line .beam {
    position: absolute; top: 0; left: -35%;
    width: 35%; height: 100%; border-radius: 10px;
    animation: beam-fwd 2s linear infinite;
  }

  .flow-arrow-line.speed .beam {
   animation: beam-fwd 1.2s linear infinite;
  }

  .flow-arrow-line.pv-beam   .beam { background: linear-gradient(90deg, transparent, #FAC775, transparent); }
  .flow-arrow-line.grid-beam .beam { background: linear-gradient(90deg, transparent, #85B7EB, transparent); }
  .flow-arrow-line.inactive          { opacity: 0.5; }
  .flow-arrow-line.inactive .beam    { display: none; }

  @keyframes beam-fwd { to { left: 135%; } }

  .contribution-bar {
    width: 100%; height: 8px; border-radius: 10px; overflow: hidden;
    background: rgba(255,255,255,0.06); display: flex; margin-top: 2px;
  }
  .contrib-pv   { height: 100%; background: #FAC775; transition: width 0.8s cubic-bezier(0.4,0,0.2,1); border-radius: 10px 0 0 10px; }
  .contrib-grid { height: 100%; background: #378ADD; transition: width 0.8s cubic-bezier(0.4,0,0.2,1); border-radius: 0 10px 10px 0; }

  .contrib-legend { display: flex; justify-content: space-between; margin-top: 5px;    width: 120px; }
  .contrib-label  { font-size: 12px; color: rgba(232,237,245,0.4); display: flex; align-items: center; gap: 4px; }
  .contrib-dot    { width: 6px; height: 6px; border-radius: 50%; }

  /* ── STATUS CHIPS ── */
  .status-row { grid-column: 1 / -1; display: flex; gap: 8px;    padding: 0px 16px; }
  .status-chip {
    flex: 1;
    background: rgba(255,255,255,0.03);
    border: 0.5px solid rgba(255,255,255,0.07);
    border-radius: 10px; padding: 10px 12px; text-align: center;
  }
  .status-chip-val { font-size: 13px; font-weight: 500; color: #e8edf5; margin-bottom: 3px; }
  .status-chip-lbl { font-size: 9px; color: rgba(232,237,245,0.35); letter-spacing: 0.8px; text-transform: uppercase; }
  
  /* LEFT SIDE AUTO PLACED (3 BOXES) */
.solar{grid-column: 1 / -2; grid-row:1; }
.ac{grid-column: 1 / -2; grid-row:2; }
.combo-card{grid-column: 1 / -1; grid-row:3; }
.contribution-block{margin-top:10px;}

.br-mobile {
  display: none;
}

.logo-text {
  display: flex;
  gap: 5px;
}

/* desktop = inline */
.line2 {
  display: inline;
}

@media (max-width: 767px) {
  .solar-dash {
    width: 100%;
  }
  .flow-section {
      grid-column: 1 / -1;
      grid-row: 1;
  }
  
  .br-mobile {
    display: inline;
  }

  .logo-text {
    flex-direction: column;
    align-items: flex-start;
  }

  .solar{grid-column: 1 / -1; grid-row:2; }
.ac{grid-column: 1 / -1; grid-row:3; }
.combo-card{grid-column: 1 / -1; grid-row:4; }
}



</style>
</head>
<body>

<div class="solar-dash">

  <!-- ── HEADER ── -->
  <div class="header">
    <div class="logo">
      <div class="logo-icon" id="headingIcon">☀</div>
      <div class="logo-text">
    Knox <span class="line2">Inverter</span>
  </div>
    </div>
    <div class="header-right">
      <div class="time-display" id="clock">--:--:--</div>
      <div class="status-pill offline" id="status_badge">
        <div class="dot red" id="status_dot"></div>
        <span id="status_text">OFFLINE</span>
      </div>
    </div>
  </div>

  <div class="main-grid">

    <!-- ── SOLAR HERO ── -->
    <div class="hero-card solar">
      <div>
        <div class="hero-label">Solar Generation</div>
        <div class="hero-value" id="h_pv_w">0 W</div>
        <div class="hero-stats">
          <div class="hero-stat">
            <div class="hero-stat-val" id="h_pv_v">0 V</div>
            <div class="hero-stat-lbl">Voltage</div>
          </div>
          <div class="hero-stat">
            <div class="hero-stat-val" id="h_pv_i">0 A</div>
            <div class="hero-stat-lbl">Ampere</div>
          </div>
        </div>
      </div>
      <div class="icon-ring solar" id="solorIcon"><div class="icon-inner">☀</div></div>
    </div>


    <!-- ── AC HERO ── -->
    <div class="hero-card ac">
      <div>
        <div class="hero-label">AC Power</div>
        <div class="hero-value" id="h_ac_w">0 W</div>
        <div class="hero-stats">
          <div class="hero-stat">
            <div class="hero-stat-val" id="h_ac_v">0 V</div>
            <div class="hero-stat-lbl">Voltage</div>
          </div>
          <div class="hero-stat">
            <div class="hero-stat-val" id="h_ac_i">0 A</div>
            <div class="hero-stat-lbl">Ampere</div>
          </div>
          <div class="hero-stat">
            <div class="hero-stat-val" id="h_ac_hz">0 Hz</div>
            <div class="hero-stat-lbl">Frequency</div>
          </div>
          <div class="hero-stat">
            <div class="hero-stat-val" id="h_ac_pf">0.00</div>
            <div class="hero-stat-lbl">PF</div>
          </div>
        </div>
      </div>
      <div class="icon-ring ac"><div class="icon-inner">⚡</div></div>
    </div>


    <!-- ── LOAD + TEMP ── -->
    <div class="combo-card">
      <div class="combo-header">
        <div class="combo-icon">📊</div>
        <div class="combo-title">System Load &amp; Temperature</div>
      </div>
      <div class="load-temp-grid">
        <div class="load-side">
          <div class="load-header">
            <div class="load-lbl">Load</div>
            <div class="load-pct-val" id="load_pct_d">0%</div>
          </div>
          <div class="load-bar-track">
            <div class="load-bar-fill" id="load_bar_d"></div>
          </div>
        </div>
        <div class="temp-side">
          <div class="temp-val" id="m_temp">0°</div>
          <div class="temp-lbl">Inverter °C</div>
        </div>
      </div>
    </div>

    	 <!-- ── ENERGY ROUTING ── -->
    <div class="flow-section">
      <div class="flow-title">Energy Routing</div>
      <div class="flow-diagram">

        <!-- Sources -->
        <div class="flow-sources">
          <div class="flow-node-box pv">
            <div class="flow-node-icon">☀</div>
            <div class="flow-node-name">PV</div>
            <div class="flow-node-watts pv-color" id="fpv_w">0 W</div>
          </div>
          <div class="flow-node-box grid">
            <div class="flow-node-icon">🔌</div>
            <div class="flow-node-name">Grid</div>
            <div class="flow-node-watts grid-color" id="fgrid_w">0 W</div>
          </div>
        </div>

        <!-- Arrows + Contribution Bar -->
        <div class="flow-middle">
          <div class="flow-arrow-row">
            <div class="flow-arrow-line pv-beam" id="arr_pv">
              <div class="beam"></div>
            </div>
          </div>
          <div class="flow-arrow-row">
            <div class="flow-arrow-line grid-beam" id="arr_grid">
              <div class="beam"></div>
            </div>
          </div>

        </div>

        <!-- Load -->
        <div class="flow-node-box load">
          <div class="flow-node-icon">🏠</div>
          <div class="flow-node-name">Load</div>
          <div class="flow-node-watts load-color" id="fload_w">0 W</div>
        </div>

      </div>
	      <!-- contribution -->
		<div class="contribution-block">
	  <div class="contribution-bar">
            <div class="contrib-pv"   id="contrib_pv"   style="width:70%"></div>
            <div class="contrib-grid" id="contrib_grid" style="width:30%"></div>
          </div>
          <div class="contrib-legend">
            <div class="contrib-label">
              <div class="contrib-dot" style="background:#FAC775"></div>
              <span id="contrib_pv_pct">70% PV</span>
            </div>
            <div class="contrib-label">
              <div class="contrib-dot" style="background:#378ADD"></div>
              <span id="contrib_grid_pct">30% Grid</span>
            </div>
          </div>
		  </div>
		  </div>
    </div>
   


    <!-- ── STATUS ── -->
    <div class="status-row">
      <div class="status-chip">
      
        <div class="status-chip-val" id="sc_mode">—</div>
        <div class="status-chip-lbl">Inverter Mode</div>
      </div>
      <div class="status-chip">
      
        <div class="status-chip-val" id="sc_mppt">—</div>
        <div class="status-chip-lbl">MPPT Status</div>
      </div>
      <div class="status-chip">
      
        <div class="status-chip-val" id="sc_conn">—</div>
        <div class="status-chip-lbl">Output Load</div>
      </div>
    </div>

  </div><!-- /.main-grid -->
</div><!-- /.solar-dash -->

<script>
  /* ── Clock ── */


  function set(id, val) {
    try {
        const el = document.getElementById(id);
        if (!el) return; // element not found, safely exit

        el.innerText = val;

    } catch (e) {
        console.log("set() error:", e);
    }
}

  /* ── Apply data to DOM ── */
  function applyData(d) {
    /* Solar */
    document.getElementById('h_pv_w').textContent = d.pv1_w + ' W';
    document.getElementById('h_pv_v').textContent = Math.round(d.pv1_v) + ' V';
    document.getElementById('h_pv_i').textContent = d.pv1_i + ' A';

    /* AC Power */
    document.getElementById('h_ac_w').textContent  = d.ac_w  + ' W';
    document.getElementById('h_ac_v').textContent  = Math.round(d.ac_v)  + ' V';
    document.getElementById('h_ac_i').textContent  = d.ac_i  + ' A';
    document.getElementById('h_ac_hz').textContent = d.ac_hz ;
    document.getElementById('h_ac_pf').textContent = parseFloat(d.ac_pf).toFixed(2);

    /* System */
    document.getElementById('m_temp').textContent    = d.temp_inv + '°';
    document.getElementById('load_pct_d').textContent = d.load_pct + '%';
    document.getElementById('load_bar_d').style.width = d.load_pct + '%';

    /* Energy routing */
    const load      = d.ac_w;
    const pvToLoad  = Math.min(d.pv1_w, load);
    const gridImp   = Math.max(0, d.grid_power_w);
    const total     = pvToLoad + gridImp || 1;
    const pvPct     = Math.round((pvToLoad / total) * 100);
    const gridPct   = 100 - pvPct;

    document.getElementById('fpv_w').textContent    = d.pv1_w   + ' W';
    document.getElementById('fgrid_w').textContent  = gridImp   + ' W';
    document.getElementById('fload_w').textContent  = load      + ' W';

    document.getElementById('contrib_pv').style.width    = pvPct   + '%';
    document.getElementById('contrib_grid').style.width  = gridPct + '%';
    document.getElementById('contrib_pv_pct').textContent   = pvPct   + '% PV';
    document.getElementById('contrib_grid_pct').textContent = gridPct + '% Grid';

    document.getElementById('arr_pv').classList.toggle('inactive',   d.pv1_w  <= 0);
    document.getElementById('arr_grid').classList.toggle('inactive', gridImp   <= 0);


    var time = formatTime(d.last_server_time);
    set("clock",time);
     
    if(d.pv1_w > gridImp){
      document.getElementById('arr_pv').classList.toggle('speed',true);
      document.getElementById('arr_grid').classList.toggle('speed',false)
    } else{
      document.getElementById('arr_pv').classList.toggle('speed',false);
      document.getElementById('arr_grid').classList.toggle('speed',true)
    }
    

    document.getElementById('solorIcon').classList.toggle('off', d.pv1_w   <= 0);

    /* Status */
    document.getElementById('sc_mode').textContent = d.inv_mode_s;
   if (d.mppt1_st == 1) {
    document.getElementById('sc_mppt').textContent = "Standby";
    } 
    else if (d.mppt1_st == 2) {
    document.getElementById('sc_mppt').textContent = "Working";
    } 
    else {
    document.getElementById('sc_mppt').textContent = "Off";
    }
   
    if(d.load_conn == 1){
     document.getElementById('sc_conn').textContent = "Connected"
    }
    else {
      document.getElementById('sc_conn').textContent = "Disconnected";
    }

    /* Online badge */
    const badge = document.getElementById('status_badge');
    const dot   = document.getElementById('status_dot');
    const txt   = document.getElementById('status_text');
    if (d.server_status) {
      badge.className = 'status-pill online';
      dot.className   = 'dot green';
      txt.textContent = 'ONLINE';
    } else {
      badge.className = 'status-pill offline';
      dot.className   = 'dot red';
      txt.textContent = 'OFFLINE';
    }

    icon.style.color = isRed ? "#795548" : "#2196F3";
    isRed = !isRed;
  }

  /* ── Mock data (shown when /data is unreachable) ── */
  function mockData() {
    return {
      pv1_w: 0, pv1_v: 0, pv1_i: 0,
      ac_w: 0,  ac_v: 0,  ac_hz: 0, ac_i:0, ac_pf: 0,
      temp_inv: 0, load_pct: 0,
      grid_power_w: 0,
      inv_mode_s: '-', mppt1_st: '-', load_conn: '-',
      server_status: false
    };
  }

  function formatTime(rawTime)
{
 
  let date = new Date((rawTime - 18000) * 1000); // REMOVE offset

  const isMobile = window.innerWidth < 768;

const months = isMobile
  ? ["Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"]
  : ["January","February","March","April","May","June","July","August","September","October","November","December"];

  let day = String(date.getDate()).padStart(2, '0');
  let month = months[date.getMonth()];
 
  let year = isMobile
  ? String(date.getFullYear()).slice(-2)
  : date.getFullYear();

  let hours = date.getHours();
  let minutes = String(date.getMinutes()).padStart(2, '0');
  let seconds = String(date.getSeconds()).padStart(2, '0');

  let ampm = hours >= 12 ? "PM" : "AM";
  hours = hours % 12;
  hours = hours ? hours : 12; // convert 0 → 12

  return `${day}-${month}-${year} ${hours}:${minutes}:${seconds} ${ampm}`;
}

  /* ── Fetch loop ── */
  async function update() {
    try {
      const res = await fetch('/data');
      if (!res.ok) throw new Error('bad response');
      applyData(await res.json());
    } catch (e) {
      /* Fallback to mock so the UI always shows something */
      
    }
  }
  const icon = document.getElementById("headingIcon");
  let isRed = true;


  update();
  setInterval(update, 30000);


let ws;
let reconnectDelay = 1000;


function connectWS() {
    ws = new WebSocket("ws://" + location.host + "/ws");

    ws.onopen = function () {
        console.log("WebSocket connected");
        reconnectDelay = 1000; // reset backoff
    };

    ws.onmessage = function (event) {
        const d = JSON.parse(event.data);
        applyData(d);
    };

    ws.onerror = function (err) {
        console.log("WebSocket error", err);
    };

    ws.onclose = function () {
        console.log("WebSocket disconnected");

        // auto reconnect
        setTimeout(() => {
            reconnectDelay = Math.min(reconnectDelay * 2, 10000); // exponential backoff
            connectWS();
        }, reconnectDelay);
    };
}

// start connection
connectWS();

</script>
</body>
</html>
)rawliteral";