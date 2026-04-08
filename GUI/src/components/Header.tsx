export default function Header() {
  return (
    <header className="header">
      <div className="header-center">
        <h1 className="header-title">GPS L1 Receiver Monitor</h1>
        <p className="header-sub header-sub-strong">
          Wavelet-Domain JEPA for GNSS Spoofing Detection
        </p>
        <div style={{ marginTop: '1rem' }}>
          <span className="header-badge">NEMESIS Dashboard</span>
        </div>
      </div>
    </header>
  );
}