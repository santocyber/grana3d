# blender CAM ops.py (c) 2021 Alain Pelletier
#
# ***** BEGIN GPL LICENSE BLOCK *****
#
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ***** END GPL LICENCE BLOCK *****

# blender operators definitions are in this file. They mostly call the functions from utils.py


import bpy
from bpy.props import *
from bpy.types import Operator
from bpy_extras.io_utils import ImportHelper

from cam import utils, pack, polygon_utils_cam, simple, gcodepath, bridges, parametric, puzzle_joinery
import shapely
from shapely.geometry import Point, LineString, Polygon
import mathutils
import math


# boolean operations for curve objects

def finger_amount(space, size):
    finger_amt = space / size
    if (finger_amt % 1) != 0:
        finger_amt = round(finger_amt) + 1
    if (finger_amt % 2) != 0:
        finger_amt = round(finger_amt) + 1
    return finger_amt


def mortise(length, thickness, finger_play, cx=0, cy=0, rotation=0):
    bpy.ops.curve.simple(align='WORLD',
                         location=(cx, cy, 0),
                         rotation=(0, 0, rotation), Simple_Type='Rectangle',
                         Simple_width=length + finger_play,
                         Simple_length=thickness, shape='3D', outputType='POLY',
                         use_cyclic_u=True,
                         handleType='AUTO', edit_mode=False)
    simple.active_name("_mortise")


def interlock_groove(length, thickness, finger_play, cx=0, cy=0, rotation=0):
    mortise(length, thickness, finger_play, 0, 0, 0)
    bpy.ops.transform.translate(value=(length / 2 - finger_play / 2, 0.0, 0.0))
    bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)
    bpy.context.active_object.rotation_euler.z = rotation
    bpy.ops.transform.translate(value=(cx, cy, 0.0))
    simple.active_name("_groove")


def interlock_twist(length, thickness, finger_play, cx=0, cy=0, rotation=0, percentage=0.5):
    mortise(length, thickness, finger_play, 0, 0, 0)
    simple.active_name("_tmp")
    mortise(length * percentage, thickness, finger_play, 0, 0, math.pi / 2)
    simple.active_name("_tmp")
    h = math.hypot(thickness, length * percentage)
    oangle = math.degrees(math.asin(length * percentage / h))
    bpy.ops.curve.simple(align='WORLD', location=(0, 0, 0), rotation=(0, 0, 0), Simple_Type='Sector',
                         Simple_startangle=90 + oangle, Simple_endangle=180 - oangle, Simple_radius=h / 2,
                         use_cyclic_u=True, edit_mode=False)
    simple.active_name("_tmp")

    bpy.ops.curve.simple(align='WORLD', location=(0, 0, 0), rotation=(0, 0, 0), Simple_Type='Sector',
                         Simple_startangle=270 + oangle, Simple_endangle=360 - oangle, Simple_radius=h / 2,
                         use_cyclic_u=True, edit_mode=False)
    simple.active_name("_tmp")

    simple.union('_tmp')
    simple.rotate(rotation)
    simple.move(x=cx, y=cy)
    simple.active_name("_groove")
    simple.remove_doubles()


def twist_line(length, thickness, finger_play, percentage, amount, distance, center=True):
    # Makes an amount of twist for the distance and  centers it
    spacing = distance / amount
    while amount > 0:
        position = spacing * amount
        interlock_twist(length, thickness, finger_play, percentage=percentage, cx=position)
        print('twistline', amount, distance, position)
        amount -= 1

    simple.join_multiple('_groove')
    simple.active_name('twist_line')
    if center:
        simple.move(x=(-distance-spacing)/2)


def twist_separator_slot(length, thickness, finger_play=0.00005, percentage=0.5):
    simple.add_rectangle(thickness+finger_play/2, length, center_y=False)
    simple.move(y=((length*percentage-finger_play/2)/2))
    simple.duplicate()
    simple.mirrory()
    simple.join_multiple('simple_rectangle')
    simple.active_name('_separator_slot')
    
    
def interlock_twist_separator(length, thickness, amount, spacing, edge_distance, finger_play=0.00005, percentage=0.5,
                              start='rounded', end='rounded'):
    amount -= 1
    base_width = 2*edge_distance+spacing*amount+thickness
    simple.add_rectangle(base_width, length-finger_play*2, center_x=False)
    simple.active_name('_base')
    twist_separator_slot(length, thickness, finger_play, percentage)
    while amount > 0:
        simple.duplicate(x=spacing)
        amount -= 1
    simple.join_multiple('_separator_slot')
    simple.move(x=edge_distance+thickness/2)
    simple.difference('_', '_base')
    simple.active_name('twist_separator')


def horizontal_finger(length, thickness, finger_play, amount, center=True):
    #   creates _wfa counterpart _wfb
    #   _wfa is centered at 0,0
    #   _wfb is _wfa offset by one length
    #   takes in the
    #   length = length of the mortise
    #   thickness = thickness of the material
    #   fingerplay = tolerance in length of the finger for smooth fit
    if center:
        for i in range(amount):
            if i == 0:
                mortise(length, thickness, finger_play, 0, thickness / 2)
                simple.active_name("_width_finger")
            else:
                mortise(length, thickness, finger_play, i * 2 * length, thickness / 2)
                simple.active_name("_width_finger")
                mortise(length, thickness, finger_play, -i * 2 * length, thickness / 2)
                simple.active_name("_width_finger")
    else:
        for i in range(amount):
            mortise(length, thickness, finger_play, length / 2 + 2 * i * length, 0)
            simple.active_name("_width_finger")

    simple.join_multiple("_width_finger")

    simple.active_name("_wfa")
    bpy.ops.object.duplicate_move(OBJECT_OT_duplicate={"linked": False, "mode": 'TRANSLATION'},
                                  TRANSFORM_OT_translate={"value": (length, 0.0, 0.0)})
    simple.active_name("_wfb")


def vertical_finger(length, thickness, finger_play, amount):
    #   creates _vfa and it's counterpart _vfb
    #   _vfa is starts at 0,0
    #   _wfb is _wfa offset vertically by one length
    #   takes in the
    #   length = length of the mortise
    #   thickness = thickness of the material
    #   fingerplay = tolerance in length of the finger for smooth fit
    #   amount = amount of fingers

    for i in range(amount):
        mortise(length, thickness, finger_play, 0, i * 2 * length + length / 2, rotation=math.pi / 2)
        simple.active_name("_height_finger")

    simple.join_multiple("_height_finger")
    simple.active_name("_vfa")
    bpy.ops.object.duplicate_move(OBJECT_OT_duplicate={"linked": False, "mode": 'TRANSLATION'},
                                  TRANSFORM_OT_translate={"value": (0, -length, 0.0)})
    simple.active_name("_vfb")


def finger_pair(name, dx=0, dy=0):
    simple.make_active(name)

    xpos = (dx / 2) * 1.006
    ypos = 1.006 * dy / 2

    bpy.ops.object.duplicate_move(OBJECT_OT_duplicate={"linked": False, "mode": 'TRANSLATION'},
                                  TRANSFORM_OT_translate={"value": (xpos, ypos, 0.0)})
    simple.active_name("_finger_pair")

    simple.make_active(name)

    bpy.ops.object.duplicate_move(OBJECT_OT_duplicate={"linked": False, "mode": 'TRANSLATION'},
                                  TRANSFORM_OT_translate={"value": (-xpos, -ypos, 0.0)})
    simple.active_name("_finger_pair")
    simple.join_multiple("_finger_pair")
    bpy.ops.object.select_all(action='DESELECT')
    return bpy.context.active_object


def create_base_plate(height, width, depth):
    #   creates blank plates for
    #   _back using width and height
    #   _side using height and depth
    #   _bottom using width and depth

    bpy.ops.curve.simple(align='WORLD', location=(0, height / 2, 0), rotation=(0, 0, 0), Simple_Type='Rectangle',
                         Simple_width=width, Simple_length=height, shape='3D', outputType='POLY',
                         use_cyclic_u=True,
                         handleType='AUTO', edit_mode=False)
    simple.active_name("_back")
    bpy.ops.curve.simple(align='WORLD', location=(0, height / 2, 0), rotation=(0, 0, 0), Simple_Type='Rectangle',
                         Simple_width=depth, Simple_length=height, shape='3D', outputType='POLY',
                         use_cyclic_u=True,
                         handleType='AUTO', edit_mode=False)
    simple.active_name("_side")
    bpy.ops.curve.simple(align='WORLD', location=(0, 0, 0), rotation=(0, 0, 0), Simple_Type='Rectangle',
                         Simple_width=width, Simple_length=depth, shape='3D', outputType='POLY',
                         use_cyclic_u=True,
                         handleType='AUTO', edit_mode=False)
    simple.active_name("_bottom")


def make_flex_pocket(length, height, finger_thick, finger_width, pocket_width):
    #   creates pockets pocket using mortise function for kerf bending
    dist = 3 * finger_width / 2
    while dist < length:
        mortise(height - 2 * finger_thick, pocket_width, 0, dist, 0, math.pi / 2)
        simple.active_name("_flex_pocket")
        dist += finger_width * 2

    simple.join_multiple("_flex_pocket")
    simple.active_name("flex_pocket")


def make_variable_flex_pocket(height, finger_thick, pocket_width, locations):
    #   creates pockets pocket using mortise function for kerf bending
    for dist in locations:
        mortise(height + 2 * finger_thick, pocket_width, 0, dist, 0, math.pi / 2)
        simple.active_name("_flex_pocket")

    simple.join_multiple("_flex_pocket")
    simple.active_name("flex_pocket")


def create_flex_side(length, height, finger_thick, top_bottom=False):
    #   assumes the base fingers were created and exist
    #   crates a flex side for mortise on curve
    #   length = length of curve
    #   height = height of side
    #   finger_length = lenght of finger or mortise
    #   finger_thick = finger thickness or thickness of material
    #   finger_tol = Play for finger 0 is very tight
    #   top_bottom = fingers on top and bottom if true, just on bottom if false
    #   flex_pocket = width of pocket on the flex side.  This is for kerf bending.
    if top_bottom:
        fingers = finger_pair("base", 0, height - finger_thick)
    else:
        simple.make_active("base")
        fingers = bpy.context.active_object
        bpy.ops.transform.translate(value=(0.0, height / 2 - finger_thick / 2 + 0.0003, 0.0))

    bpy.ops.curve.simple(align='WORLD', location=(length / 2 + 0.00025, 0, 0), rotation=(0, 0, 0),
                         Simple_Type='Rectangle', Simple_width=length, Simple_length=height, shape='3D',
                         outputType='POLY', use_cyclic_u=True, handleType='AUTO', edit_mode=False)
    simple.active_name("no_fingers")

    bpy.ops.curve.simple(align='WORLD', location=(length / 2 + 0.00025, 0, 0), rotation=(0, 0, 0),
                         Simple_Type='Rectangle', Simple_width=length, Simple_length=height, shape='3D',
                         outputType='POLY', use_cyclic_u=True, handleType='AUTO', edit_mode=False)
    simple.active_name("_side")

    simple.make_active('_side')
    fingers.select_set(True)
    bpy.ops.object.curve_boolean(boolean_type='DIFFERENCE')

    simple.active_name("side")
    simple.remove_multiple('_')
    simple.remove_multiple('base')


def angle(a, b):
    return math.atan2(b[1] - a[1], b[0] - a[0])


def angle_difference(a, b, c):
    return angle(a, b) - angle(b, c)


def fixed_finger(loop, loop_length, finger_size, finger_thick, finger_tolerance, base=False):
    #   distributes mortises of a fixed distance
    #   dynamically changes the finger tolerance with the angle differences
    #   loop = takes in a shapely shape
    #   finger_size = size of the mortise
    #   finger_thick = thickness of the material
    #   finger_tolerance = minimum finger tolerance

    coords = list(loop.coords)
    old_mortise_angle = 0
    distance = finger_size / 2
    j = 0
    print("joinery loop length", round(loop_length * 1000), "mm")
    for i, p in enumerate(coords):
        if i == 0:
            p_start = p

        if p != p_start:
            not_start = True
        else:
            not_start = False
        pd = loop.project(Point(p))

        if not_start:
            while distance <= pd:
                mortise_angle = angle(oldp, p)
                mortise_angle_difference = abs(mortise_angle - old_mortise_angle)
                mad = (1 + 6 * min(mortise_angle_difference, math.pi / 4) / (
                        math.pi / 4))  # factor for tolerance for the finger

                if base:
                    mortise(finger_size, finger_thick, finger_tolerance * mad, distance, 0, 0)
                    simple.active_name("_base")
                else:
                    mortise_point = loop.interpolate(distance)
                    mortise(finger_size, finger_thick, finger_tolerance * mad, mortise_point.x, mortise_point.y,
                            mortise_angle)

                j += 1
                distance = j * 2 * finger_size + finger_size / 2
                old_mortise_angle = mortise_angle
        oldp = p
    if base:
        simple.join_multiple("_base")
        simple.active_name("base")
        simple.move(x=finger_size)
    else:
        simple.join_multiple("_mort")
        simple.active_name("mortise")


def find_slope(p1, p2):
    return (p2[1] - p1[1]) / max(p2[0] - p1[0], 0.00001)


def slope_array(loop):
    simple.remove_multiple("-")
    coords = list(loop.coords)
    #    pnt_amount = round(length / resolution)
    sarray = []
    dsarray = []
    for i, p in enumerate(coords):
        distance = loop.project(Point(p))
        if i != 0:
            slope = find_slope(p, oldp)
            sarray.append((distance, slope * -0.001))
        oldp = p
    for i, p in enumerate(sarray):
        distance = p[0]
        if i != 0:
            slope = find_slope(p, oldp)
            if abs(slope) > 10:
                print(distance)
            dsarray.append((distance, slope * -0.00001))
        oldp = p
    derivative = LineString(sarray)
    dderivative = LineString(dsarray)
    utils.shapelyToCurve('-derivative', derivative, 0.0)
    utils.shapelyToCurve('-doublederivative', dderivative, 0.0)
    return sarray


def dslope_array(loop, resolution=0.001):
    length = loop.length
    pnt_amount = round(length / resolution)
    sarray = []
    dsarray = []
    for i in range(pnt_amount):
        distance = i * resolution
        pt = loop.interpolate(distance)
        p = (pt.x, pt.y)
        if i != 0:
            slope = abs(angle(p, oldp))
            sarray.append((distance, slope * -0.01))
        oldp = p
    #    derivative = LineString(sarray)
    for i, p in enumerate(sarray):
        distance = p[0]
        if i != 0:
            slope = find_slope(p, oldp)
            if abs(slope) > 10:
                print(distance)
            dsarray.append((distance, slope * -0.1))
        oldp = p
    dderivative = LineString(dsarray)
    utils.shapelyToCurve('doublederivative', dderivative, 0.0)
    return sarray


def variable_finger(loop, loop_length, min_finger, finger_size, finger_thick, finger_tolerance, adaptive, base=False,
                    double_adaptive=False):
    #   distributes mortises of a fixed distance
    #   dynamically changes the finger tolerance with the angle differences
    #   loop = takes in a shapely shape
    #   finger_size = size of the mortise
    #   finger_thick = thickness of the material
    #   finger_tolerance = minimum finger tolerance
    #   adaptive = angle threshold to reduce finger size

    coords = list(loop.coords)
    old_mortise_angle = 0
    distance = min_finger / 2
    finger_sz = min_finger
    oldfinger_sz = min_finger
    hpos = []  # hpos is the horizontal positions of the middle of the mortise
    # slope_array(loop)
    print("joinery loop length", round(loop_length * 1000), "mm")
    for i, p in enumerate(coords):
        if i == 0:
            p_start = p

        if p != p_start:
            not_start = True
        else:
            not_start = False
        pd = loop.project(Point(p))

        if not_start:
            while distance <= pd:
                mortise_angle = angle(oldp, p)
                mortise_angle_difference = abs(mortise_angle - old_mortise_angle)
                mad = (1 + 6 * min(mortise_angle_difference, math.pi / 4) / (
                        math.pi / 4))  # factor for tolerance for the finger
                distance += mad * finger_tolerance  # move finger by the factor mad greater with larger angle difference
                mortise_point = loop.interpolate(distance)
                if mad > 2 and double_adaptive:
                    hpos.append(distance)  # saves the mortise center

                hpos.append(distance + finger_sz)  # saves the mortise center
                if base:
                    mortise(finger_sz, finger_thick, finger_tolerance * mad, distance + finger_sz, 0, 0)
                    simple.active_name("_base")
                else:
                    mortise(finger_sz, finger_thick, finger_tolerance * mad, mortise_point.x, mortise_point.y,
                            mortise_angle)
                    if i == 1:
                        #  put a mesh cylinder at the first coordinates to indicate start
                        simple.remove_multiple("start_here")
                        bpy.ops.mesh.primitive_cylinder_add(radius=finger_thick / 2, depth=0.025, enter_editmode=False,
                                                            align='WORLD',
                                                            location=(mortise_point.x, mortise_point.y, 0),
                                                            scale=(1, 1, 1))
                        simple.active_name("start_here_mortise")

                old_distance = distance
                old_mortise_point = mortise_point
                finger_sz = finger_size
                next_angle_difference = math.pi

                #   adaptive finger length start
                while finger_sz > min_finger and next_angle_difference > adaptive:
#                while finger_sz > min_finger and next_angle_difference > adaptive:
                    finger_sz *= 0.95  # reduce the size of finger by a percentage... the closer to 1.0, the slower
                    distance = old_distance + 3 * oldfinger_sz / 2 + finger_sz / 2
                    mortise_point = loop.interpolate(distance)  # get the next mortise point
                    next_mortise_angle = angle((old_mortise_point.x, old_mortise_point.y),
                                               (mortise_point.x, mortise_point.y))  # calculate next angle
                    next_angle_difference = abs(next_mortise_angle - mortise_angle)

                oldfinger_sz = finger_sz
                old_mortise_angle = mortise_angle
        oldp = p
    if base:
        simple.join_multiple("_base")
        simple.active_name("base")
    else:
        print("placeholder")
        simple.join_multiple("_mort")
        simple.active_name("variable_mortise")
    return hpos


def single_interlock(finger_depth, finger_thick, finger_tolerance, x, y, groove_angle, type, amount=1,
                     twist_percentage=0.5):
    if type == "GROOVE":
        interlock_groove(finger_depth, finger_thick, finger_tolerance, x, y, groove_angle)
    elif type == "TWIST":
        interlock_twist(finger_depth, finger_thick, finger_tolerance, x, y, groove_angle, percentage=twist_percentage)
    elif type == "PUZZLE":
        puzzle_joinery.fingers(finger_thick, finger_tolerance)


def distributed_interlock(loop, loop_length, finger_depth, finger_thick, finger_tolerance, finger_amount, tangent=0,
                          fixed_angle=0, start=0.01, end=0.01, closed=True, type='GROOVE', twist_percentage=0.5):
    #   distributes interlocking joints of a fixed amount
    #   dynamically changes the finger tolerance with the angle differences
    #   loop = takes in a shapely shape
    #   finger_size = size of the mortise
    #   finger_thick = thickness of the material
    #   finger_tolerance = minimum finger tolerance
    #   twist_percentage = portion of twist finger which is the stem
    coords = list(loop.coords)
    print(closed)
    if not closed:
        spacing = (loop_length - start - end) / (finger_amount-1)
        distance = start
        end_distance = loop_length - end
    else:
        spacing = loop_length / finger_amount
        distance = 0
        end_distance = loop_length

    j = 0
    print("joinery loop length", round(loop_length * 1000), "mm")
    print("distance between joints", round(spacing * 1000), "mm")

    for i, p in enumerate(coords):
        if i == 0:
            p_start = p

        if p != p_start:
            not_start = True
        else:
            not_start = False
        pd = loop.project(Point(p))

        if not_start:
            while distance <= pd and end_distance >= distance:
                if fixed_angle == 0:
                    groove_angle = angle(oldp, p) + math.pi / 2 + tangent
                else:
                    groove_angle = fixed_angle

                groove_point = loop.interpolate(distance)

                print(j, "groove_angle", round(180 * groove_angle / math.pi), "distance", round(distance * 1000), "mm")
                single_interlock(finger_depth, finger_thick, finger_tolerance, groove_point.x, groove_point.y,
                                 groove_angle, type, twist_percentage=twist_percentage)

                j += 1
                distance = j * spacing + start
        oldp = p

    simple.join_multiple("_groove")
    simple.active_name("interlock")
