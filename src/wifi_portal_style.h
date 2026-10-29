#pragma once

static const char PORTAL_STYLE[] = R"rawliteral(
<meta name="theme-color" content="#0E1116">
<style>
  body {
    background: #0E1116;
    color: #FFFFFF;
    font-family: -apple-system, system-ui, "Segoe UI", Roboto, sans-serif;
  }
  .wrap { max-width: 420px; padding: 24px 16px; }

  h1 { color: #4FC3F7; font-weight: 800; letter-spacing: .5px; }
  h3 { color: #8B95A5; font-weight: 500; }
  a  { color: #4FC3F7; }
  hr { border: 0; border-top: 1px solid #252C37; }
  label { color: #8B95A5; font-size: 13px; }

  input, select {
    background: #171C24;
    color: #FFFFFF;
    border: 1px solid #252C37;
    border-radius: 14px;
    padding: 14px 16px;
    font-size: 16px;
    box-sizing: border-box;
  }
  input:focus, select:focus { outline: none; border-color: #4FC3F7; }
  input[type=checkbox] { width: auto; accent-color: #4FC3F7; }

  button, input[type=submit] {
    background: #4FC3F7;
    color: #000000;
    border: 0;
    border-radius: 14px;
    font-size: 16px;
    font-weight: 700;
    line-height: 52px;
    cursor: pointer;
  }
  button:hover, input[type=submit]:hover { filter: brightness(1.1); }
  button.D { background: #EF5350; color: #FFFFFF; }

  .wrap > div > a[onclick] { color: #FFFFFF; font-weight: 600; text-decoration: none; }
  .q { filter: invert(1); opacity: .85; }

  .msg {
    background: #171C24;
    color: #FFFFFF;
    border: 0;
    border-left: 5px solid #4FC3F7;
    border-radius: 14px;
  }
  .msg.S { border-left-color: #66BB6A; }
  .msg.D { border-left-color: #EF5350; }
  .msg.P { border-left-color: #4FC3F7; }

  dt { color: #8B95A5; font-weight: 500; }
  dd { color: #FFFFFF; margin-bottom: 8px; }
  table { color: #FFFFFF; }
</style>
)rawliteral";