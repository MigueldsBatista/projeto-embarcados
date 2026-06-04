export interface Card {
  id: number;
  uid: string;
  name: string;
  role: string;
  is_active: boolean;
  created_at: string;
}

export interface AccessLog {
  id: number;
  uid: string;
  status: 'authorized' | 'unknown' | 'inactive';
  timestamp: string;
}

export interface Summary {
  total_scans: number;
  unknown_scans: number;
}
