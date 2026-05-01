# Maze Tilt Game (Adjusted Boundary Thickness)
# Run with: pgzrun filename.py

import pgzrun
import serial

ser = serial.Serial('/dev/tty.usbmodem2101')

# --------------------------------------------------
# WINDOW
# --------------------------------------------------
WIDTH = 800
HEIGHT = 600
TITLE = "Maze Tilt Game"

# --------------------------------------------------
# SETTINGS
# --------------------------------------------------
BALL_RADIUS = 12
BALL_START = (60, 60)

MAX_TILT = 0.35
FRICTION = 0.98

BOUNDARY_THICKNESS = 20   # <-- half thickness (cleaner feel)

# --------------------------------------------------
# BALL STATE
# --------------------------------------------------
ball_x = BALL_START[0]
ball_y = BALL_START[1]
ball_vx = 0
ball_vy = 0

won = False
lost = False

# --------------------------------------------------
# GOAL
# --------------------------------------------------
goal = Rect((730, 520), (40, 40))

# --------------------------------------------------
# MAZE (unchanged)
# --------------------------------------------------
walls = [
    Rect((130, 0), (20, 430)),
    Rect((260, 160), (20, 440)),
    Rect((400, 0), (20, 360)),
    Rect((540, 220), (20, 380)),
    Rect((670, 0), (20, 430)),

    Rect((175, 120), (55, 20)),
    Rect((175, 260), (70, 20)),

    Rect((305, 420), (70, 20)),
    Rect((315, 250), (55, 20)),

    Rect((445, 110), (70, 20)),
    Rect((445, 260), (60, 20)),

    Rect((585, 420), (65, 20)),
    Rect((585, 290), (55, 20)),

    Rect((95, 500), (35, 35)),
    Rect((350, 90), (35, 35)),
    Rect((610, 140), (35, 35)),
]

# --------------------------------------------------
# INPUT
# --------------------------------------------------
def get_tilt_input():

    n_bytes = ser.readline() # read all the letters available
    s = str(n_bytes) # turn them into a str
    result1 = s[s.find('(')+1:s.find(',')] # find everything beween ( and ,
    result2 = s[s.find(',')+1:s.find(')')] # find everything between , and )
    global got_accelx
    got_accelx = float(result1)/100000000.0 # convert str to float
    global accel_y
    got_accely = float(result2)/100000000.0

    tx = 0
    ty = 0

    if got_accelx>0:
        tx = -1
    elif got_accelx<0:
        tx = 1

    if got_accely>0:
        ty = -1
    elif got_accely<0:
        ty = 1

    return tx, ty


# --------------------------------------------------
# RESET
# --------------------------------------------------
def reset_game():
    global ball_x, ball_y, ball_vx, ball_vy, won, lost
    ball_x, ball_y = BALL_START
    ball_vx = 0
    ball_vy = 0
    won = False
    lost = False


# --------------------------------------------------
# COLLISION
# --------------------------------------------------
def ball_hits_rect(cx, cy, r, rect):
    nx = max(rect.left, min(cx, rect.right))
    ny = max(rect.top, min(cy, rect.bottom))
    dx = cx - nx
    dy = cy - ny
    return dx*dx + dy*dy < r*r



# --------------------------------------------------
# UPDATE
# --------------------------------------------------
def update():
    global ball_x, ball_y, ball_vx, ball_vy, won

    if keyboard.r:
        reset_game()

    if won or lost:
        return

    tx, ty = get_tilt_input()

    ball_vx += tx * 0.35
    ball_vy += ty * 0.35

    ball_vx *= 0.98
    ball_vy *= 0.98

    ball_x += ball_vx

    for w in walls:
        if ball_hits_rect(ball_x, ball_y, BALL_RADIUS, w):
            ball_x -= ball_vx
            ball_vx *= -0.3
            break

    ball_y += ball_vy

    for w in walls:
        if ball_hits_rect(ball_x, ball_y, BALL_RADIUS, w):
            ball_y -= ball_vy
            ball_vy *= -0.3
            break

    if not lost and ball_hits_rect(ball_x, ball_y, BALL_RADIUS, goal):
        won = True


# --------------------------------------------------
# DRAW
# --------------------------------------------------
def draw():
    screen.fill((230, 230, 230))

    # thin red danger border
    screen.draw.filled_rect(Rect((0, 0), (WIDTH, BOUNDARY_THICKNESS)), (220, 0, 0))
    screen.draw.filled_rect(Rect((0, HEIGHT-BOUNDARY_THICKNESS), (WIDTH, BOUNDARY_THICKNESS)), (220, 0, 0))
    screen.draw.filled_rect(Rect((0, 0), (BOUNDARY_THICKNESS, HEIGHT)), (220, 0, 0))
    screen.draw.filled_rect(Rect((WIDTH-BOUNDARY_THICKNESS, 0), (BOUNDARY_THICKNESS, HEIGHT)), (220, 0, 0))

    for w in walls:
        screen.draw.filled_rect(w, (60, 60, 60))

    screen.draw.filled_rect(goal, (0, 180, 0))
    screen.draw.text("GOAL", center=goal.center, color="white")

    screen.draw.filled_circle((ball_x, ball_y), BALL_RADIUS, (30, 100, 255))

    screen.draw.text(
        "Arrow Keys = Tilt | R = Restart | Touch Red = Lose",
        (20, 10),
        fontsize=26,
        color="black"
    )

    if won:
        screen.draw.text("YOU WIN!", center=(WIDTH//2, HEIGHT//2), fontsize=70, color="green")

    if lost:
        screen.draw.text("YOU LOSE!", center=(WIDTH//2, HEIGHT//2), fontsize=70, color="red")


pgzrun.go()