import tc.wata.debug.*;

public class P implements Comparable<P> {
	
	public R x, y;
	
	public P(R x, R y) {
		this.x = x;
		this.y = y;
	}
	
	public P(int x, int y) {
		this(new R(x), new R(y));
	}
	
	public P add(P p) {
		return new P(x.add(p.x), y.add(p.y));
	}
	
	public P sub(P p) {
		return new P(x.sub(p.x), y.sub(p.y));
	}
	
	public P mul(R d) {
		return new P(x.mul(d), y.mul(d));
	}
	
	public P div(R d) {
		return new P(x.div(d), y.div(d));
	}
	
	public R dot(P p) {
		return x.mul(p.x).add(y.mul(p.y));
	}
	
	public R det(P p) {
		return x.mul(p.y).sub(y.mul(p.x));
	}
	
	public R abs2() {
		return dot(this);
	}
	
	public int compareTo(P o) {
		int comp = x.compareTo(o.x);
		if (comp != 0) return comp;
		return y.compareTo(o.y);
	}
	
	public String toString() {
		return String.format("(%s,%s)", x, y);
	}
	
	public static P isLL(P p1, P p2, P q1, P q2) {
		R d = q2.sub(q1).det(p2.sub(p1));
		if (d.signum() == 0) return null;
		return p1.add(p2.sub(p1).mul(q2.sub(q1).det(q1.sub(p1)).div(d)));
	}
	
	public static boolean crsSS(P p1, P p2, P q1, P q2) {
		R det = p2.sub(p1).det(q2.sub(q1));
		if (det.signum() == 0) {
			if (p2.sub(p1).det(q1.sub(p1)).signum() == 0) {
				if (p1.sub(q1).dot(p2.sub(q1)).signum() < 0) return true;
				if (p1.sub(q2).dot(p2.sub(q2)).signum() < 0) return true;
				if (q1.sub(p1).dot(q2.sub(p1)).signum() < 0) return true;
				if (q1.sub(p2).dot(q2.sub(p2)).signum() < 0) return true;
			}
			return false;
		}
		return p2.sub(p1).det(q1.sub(p1)).signum() * p2.sub(p1).det(q2.sub(p1)).signum() < 0 &&
			   q2.sub(q1).det(p1.sub(q1)).signum() * q2.sub(q1).det(p2.sub(q1)).signum() < 0;
	}
	
	public static boolean crsPP(P[] ps, P[] qs) {
		try (Stat st = new Stat("srcPP")) {
		for (int i = 0; i < ps.length; i++) {
			if (contains(qs, ps[i].add(ps[(i + 1) % ps.length]).div(R.TWO)) > 0) return true;
			for (int j = 0; j < qs.length; j++) {
				if (crsSS(ps[(i + 1) % ps.length], ps[i], qs[(j + 1) % qs.length], qs[j])) return true;
			}
		}
		}
		return false;
	}
	
	public static int contains(P[] ps, P q) {
		int n = ps.length;
		int res = -1;
		for (int i = 0; i < n; i++) {
			P a = ps[i].sub(q), b = ps[(i + 1) % n].sub(q);
			if (a.y.compareTo(b.y) > 0) {
				P t = a; a = b; b = t;
			}
			if (a.y.signum() <= 0 && b.y.signum() > 0 && a.det(b).signum() > 0) {
				res = -res;
			}
			if (a.det(b).signum() == 0 && a.dot(b).signum() <= 0) return 0;
		}
		return res;
	}
	
	// aをbの方向に回転
	public static R[][] rotate(P a, P b) {
		R ab = a.abs2().mul(b.abs2()).sqrt();
		if (ab == null) return null;
		R cos = a.dot(b).div(ab), sin = a.det(b).div(ab);
		return new R[][]{{cos, sin.neg()}, {sin, cos}};
	}
	
	// d方向を軸に反転
	public static R[][] reflect(P d) {
		R d2 = d.dot(d);
		R cos = d.x.mul(d.x).sub(d.y.mul(d.y)).div(d2), sin = R.TWO.mul(d.x).mul(d.y).div(d2);
		return new R[][]{{cos, sin}, {sin, cos.neg()}};
	}
	
	public static P[] apply(R[][] A, P[] ps) {
		P[] qs = new P[ps.length];
		for (int i = 0; i < ps.length; i++) {
			qs[i] = new P(A[0][0].mul(ps[i].x).add(A[0][1].mul(ps[i].y)), A[1][0].mul(ps[i].x).add(A[1][1].mul(ps[i].y)));
		}
		return qs;
	}
	
	public static P[] add(P[] ps, P q) {
		P[] qs = new P[ps.length];
		for (int i = 0; i < ps.length; i++) qs[i] = ps[i].add(q);
		return qs;
	}
	
	public static R area(P[] ps) {
		R area = R.ZERO;
		for (int i = 0; i < ps.length; i++) {
			area = area.add(ps[i].det(ps[(i + 1) % ps.length]));
		}
		return area.div(R.TWO);
	}
	
	public static double rad(P p, P q) {
		double th = Math.atan2(p.det(q).getDouble(), p.dot(q).getDouble());
		if (th < 0) th += 2 * Math.PI;
		return th;
	}
	
}