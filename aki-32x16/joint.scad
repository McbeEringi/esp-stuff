$fa=5;$fs=.2;

t=1.8;
l=4;
d=8.2;
r=7/2;

module j(hole_d){
	difference(){
		minkowski(){circle(r=r);square([d,1e-9],center=true);}
		translate([ d/2,0])circle(d=hole_d);
		translate([-d/2,0])circle(d=hole_d);
	}
}

linear_extrude(l-t-.5)j(3);
linear_extrude(.5)translate([0,10])j(3.2);