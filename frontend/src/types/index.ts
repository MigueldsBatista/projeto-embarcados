export interface Card {
  id: number;
  uid: string;
  name: string;
  role: string;
  is_active: boolean;
  tracker_mac?: string;
  created_at: string;
}

export interface TrackingStatus {
  student_name: string;
  mac: string;
  status: 'online' | 'offline';
  rssi: number | null;
  zone: 'Very Near' | 'Near' | 'Far' | 'Unknown';
  last_seen: string | null;
}

export interface DiscoveredTag {
  rssi: number;
  timestamp: string;
}

export interface AccessLog {
  id: number;
  uid: string;
  status: 'authorized' | 'unknown' | 'inactive';
  timestamp: string;
  person_name?: string;
}

export interface Summary {
  total_scans: number;
  unknown_scans: number;
}
