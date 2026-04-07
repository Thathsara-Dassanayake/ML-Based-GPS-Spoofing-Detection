import React from 'react';

type Props = {
  status: "Legitimate" | "Spoofed" | "Inactive";
};

export default function StatusPanel({ status }: Props) {
  let statusClass = "status-inactive";
  let statusText = "INACTIVE - Select a Signal to Acquire";

  if (status === "Legitimate") {
    statusClass = "status-legitimate";
    statusText = "TRACKING: LEGITIMATE";
  } else if (status === "Spoofed") {
    statusClass = "status-spoofed";
    statusText = "DANGER: SPOOFED SIGNAL DETECTED";
  }

  return (
    <div className={`status-panel-beautiful ${statusClass}`}>
      <div className="status-indicator"></div>
      <h2 className="status-text">{statusText}</h2>
    </div>
  );
}
