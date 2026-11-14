from pathlib import Path
from docx import Document
from docx.shared import Inches, Pt, RGBColor
from docx.oxml.ns import qn

OUTPUT = Path(__file__).resolve().parents[1] / "docs" / "SmartFlow_Technical_Showcase_Group_Script.docx"
doc = Document()
section = doc.sections[0]
section.top_margin = Inches(.72)
section.bottom_margin = Inches(.68)
section.left_margin = Inches(.82)
section.right_margin = Inches(.82)
normal = doc.styles["Normal"]
normal.font.name = "Aptos"
normal.font.size = Pt(10.5)
normal.font.color.rgb = RGBColor(20, 30, 34)
normal.paragraph_format.space_after = Pt(6)
normal.paragraph_format.line_spacing = 1.12
for name, size in [("Title", 20), ("Heading 1", 14), ("Heading 2", 11)]:
    style = doc.styles[name]
    style.font.name = "Aptos"
    style.font.size = Pt(size)
    style.font.color.rgb = RGBColor(0, 0, 0)
    style.paragraph_format.space_before = Pt(12)
    style.paragraph_format.space_after = Pt(5)
    ppr = style.element.get_or_add_pPr()
    border = ppr.find(qn("w:pBdr"))
    if border is not None:
        ppr.remove(border)

def heading(title):
    doc.add_heading(title, 1)

def speak(text):
    doc.add_paragraph(text)

def operator(text):
    p = doc.add_paragraph()
    p.paragraph_format.space_before = Pt(7)
    p.paragraph_format.space_after = Pt(7)
    run = p.add_run("SCREEN SHARER  " + text)
    run.bold = True
    run.font.size = Pt(9.5)

title = doc.add_paragraph("SmartFlow Group Technical Showcase Speaking Script", "Title")
speak("This is the recording script for the eleven sections in SmartFlow's Presentation tab. The words below are written to be spoken, not read as a report. One member shares the screen and scrolls only at the marked cues. Each speaker should rehearse their own explanation, point to the image and code reference on screen, and use their normal voice. Keep every camera visible beside the shared screen.")
speak("Presentation order: Grace (sections 1 and 2), Elijah (3 and 4), Billiart (5 to 8), Kevin (9 and 10), Samuel (11). Then the screen sharer demonstrates the running Simulation and Graphics lab and opens the current GitHub repository. If a screenshot or repository detail changes before recording, describe the live version you can actually show.")

heading("Preparation before recording")
operator("Open the freshly built SmartFlow application. Press F11 if fullscreen helps the recording. Select Presentation and scroll to the top. Open the current GitHub repository in a browser behind the app. Check that the main branch includes the work you plan to mention. Keep the Simulation paused until the live segment.")
speak("Agree on one person to control the screen. Test the microphone levels and recording layout: the lecturer must be able to see all members' cameras and the shared screen. Do one dry run of the scrolling and the live controls. The screenshots support the talk; they are not a replacement for the actual app demonstration.")

heading("Grace  Section 1  Why we chose a traffic junction")
operator("Show 1 of 11. Hold the first section still while Grace introduces the project.")
speak("Good day. We are presenting SmartFlow, our C++ traffic simulation. When we discussed the project, we wanted to use the graphical representations we learned in class to model a system people would recognise. We chose a traffic junction because we can see several computer graphics ideas in one place: positions change over time, objects have direction, paths turn, and a signal changes what is allowed to move.")
speak("The idea was to create a junction where we could vary traffic demand and signal time and observe different situations. For example, if more vehicles arrive, we can see queues form. If a signal changes, vehicles must respond to it. The program is interactive so we can compare those situations instead of showing one fixed animation.")
speak("We set the scope at four approaches and three possible movements from each approach: straight, left and right. That gives twelve routes. We used left-hand traffic throughout, so the lane positions, stop lines and turns have to agree with one another. The image on the right shows the scene built from those decisions. My role was to frame the purpose and scope so the code and the visual result would answer the same project question.")

heading("Grace  Section 2  Proposal compared with the actual project")
operator("Scroll one section to 2 of 11. Point to the migration note and the traffic screenshot when Grace mentions the change of language.")
speak("Our early prototype used SVG and JavaScript. When the requirement was clarified as C++, we did not just rename the files: we made a native C++17 version. Raylib supplies the window, input events and basic drawing functions. Our C++ source calculates the routes, traffic behaviour and graphics algorithm results that raylib displays.")
speak("We also had to decide what would fit within the project. Pedestrian crossings sounded useful, but they raised several questions: where would people appear, which crossing would they use, would they press a button, how long would the walk phase last, and how would that phase interact with cars already inside the junction? We considered multiple linked intersections too, but that would require coordinated signal timing between junctions. We left incident scenarios outside this version rather than show an animation without rules behind it.")
speak("The actual work kept the central traffic idea and delivered one interactive junction, twelve routes, timed signals, a graphics lab and tests. The difficult part of the scope decision was accepting that each visible feature also needs a model, controls and evidence that it behaves correctly. I now hand over to Elijah to explain the model that makes the cars and signals work.")

heading("Elijah  Section 3  A car is data before it is a drawing")
operator("Scroll to 3 of 11. Keep the model section and running traffic screenshot visible. Point to the source card on the lower right when Elijah names the functions.")
speak("I worked on the traffic model and its integration with the application. Before a car is drawn, it is a Vehicle in C++ with an identifier, a selected route, distance travelled along that route, speed, position, direction and time spent waiting. The model also holds the current signal phase, demand setting, pending arrivals and statistics. That state is the source of truth for the display.")
speak("In simulation.cpp, step(dt) receives elapsed time from the application. It accumulates that time and repeatedly calls fixedStep() at the model's fixed interval. Within a fixed step, the program advances the signals, handles new arrivals, moves cars, removes completed trips and refreshes the statistics. This means the result does not depend on whether the screen happened to draw thirty or sixty frames in a second.")
speak("Arrivals are not placed on top of another car. They wait in a pending list until the route entry is clear. For movement, the model processes cars farther along their routes first and checks the distance to vehicles ahead. Each car's route progress is converted into x and y coordinates and a heading. The renderer receives that calculated state; it does not decide where the car should go. The challenge was keeping the simulation consistent while the UI changes speed or pauses, so model time and drawing time were kept separate.")

heading("Elijah  Section 4  Signals queues and safe movement")
operator("Scroll to 4 of 11. Point first to the signal sequence in the code card, then to the signal lights and the statistics in the screenshot.")
speak("The signal controller is a small state machine. An approach receives green, then amber, then an all-red clearance period. Only after the clearance period, and after the junction is no longer occupied, can the next approach become green. Pressing Next phase starts the safe transition; it does not jump straight to another green. This is why the signal test checks that repeated button presses cannot skip amber or all-red.")
speak("A car nearing a red light targets the stop line and reduces its speed as the available stopping distance decreases. Cars also check a safe gap to a leader. Near turns and merges, the model checks whether the proposed new position overlaps another car; if it does, it shortens the movement. We deliberately favour a conservative stop over cars crossing through each other. Once a car has entered under a green signal, the clearance logic lets it leave the junction before conflicting traffic receives green.")
speak("The controls are connected to this model. Traffic demand changes the spacing of new arrivals. A change to green duration becomes active at the next green phase. Start and Pause control time, Step advances one simulated second while paused, and Reset restores a seeded scenario. The dashboard reports active vehicles, queued vehicles, completed trips and average wait. We also added Fit, which enlarges the simulation screen for the live demonstration. Next, Billiart will explain how the paths and graphics calculations are represented.")

heading("Billiart  Section 5  Curves sampling and transformations")
operator("Scroll to 5 of 11. Point to the route overlay screenshot and the two source expressions.")
speak("My section covers the geometry behind the routes and the three calculations in the graphics lab. We have four approaches and three turn choices, so the C++ route generator creates twelve paths. It starts with a base path and rotates its control points around the centre of the junction to produce the corresponding routes from the other directions. This keeps the geometry consistent instead of drawing twelve unrelated paths by eye.")
speak("For a turn, cubic Bezier geometry gives a smooth curve from four control points. The code samples that curve at regular parameter intervals and stores the cumulative distance between samples. A vehicle moves according to distance along the path, not by jumping from one drawing point to another. pointAt(progress) finds the neighbouring samples, interpolates a position and uses atan2 on their direction to calculate the heading.")
speak("The visual transformation follows directly: translate the car shape to the calculated x and y position, then rotate it by the heading angle. The route overlay in this screenshot helps us inspect the paths. Our challenge was preventing sudden jumps or sharp changes in direction; the route tests check continuity, start and end positions, and heading changes across all twelve paths.")

heading("Billiart  Section 6  Bresenham line rasterisation")
operator("Scroll to 6 of 11. Point to the green square samples and the gold reference line.")
speak("Bresenham's line algorithm answers a precise graphics question: given two integer endpoints, which grid cells should form a connected line? We implemented it in graphics.cpp. The code calculates horizontal and vertical differences, step directions, and an error value. At each iteration it adds the current pixel, then uses the error to decide whether x, y or both should change.")
speak("The differences are widened before subtraction so extreme integer coordinates do not overflow during that calculation. The same loop works for shallow and steep lines in every direction. That was the challenge: we did not want an implementation that only worked when the line went down and right. The tests cover horizontal, vertical, steep and reversed cases.")
speak("The green squares are the pixels returned by our algorithm. The gold line is an ideal geometric reference drawn for comparison. We will simulate a change in the graph during the live demonstration so you can see the pixels recalculated from the new input.")

heading("Billiart  Section 7  Midpoint circle")
operator("Scroll to 7 of 11. Let the audience see the full circle image before continuing.")
speak("For a circle, calculating each pixel independently from an angle would be unnecessary. Our midpoint circle algorithm starts at one axis with a decision value. As it advances through one octant, it chooses whether to remain at the same x value or move inward. Each chosen point is mirrored to the other seven octants around the selected centre.")
speak("At the axes and diagonals, some mirrored coordinates can coincide. The implementation keeps a set of pixel keys so those duplicates appear only once in the returned samples. The result is the green ring on the graph. The gold circle is a smooth reference, which makes the difference between discrete raster samples and ideal geometry visible. In the live lab we can change the centre or radius and watch the result update.")

heading("Billiart  Section 8  Cohen Sutherland line clipping")
operator("Scroll to 8 of 11. Point to the green rectangle, the complete gold line and the accepted green segment.")
speak("Clipping asks which part of a line should remain visible inside a rectangular boundary. Cohen-Sutherland gives each endpoint a four-bit region code that tells us whether it is left, right, above or below the rectangle. If both codes are zero, the whole line is accepted. If the codes share an outside bit, the whole line can be rejected. Otherwise, the program finds an intersection with one boundary, replaces the outside endpoint and repeats.")
speak("In our lab the gold line shows the original segment, and green shows the part accepted inside the rectangle. We test completely inside, completely outside and partly crossing cases. The traffic renderer also clips the scene to its viewport, although that viewport clipping is a separate drawing operation from this classroom algorithm. This distinction matters: the lab demonstrates our algorithm, while the renderer uses clipping to keep offscreen cars out of the margins. Kevin will now show how those calculated states become the visible junction.")

heading("Kevin  Section 9  Drawing the junction")
operator("Scroll to 9 of 11. Point to the road, crossing, signals and buildings in the enlarged-scene image.")
speak("My contribution is the scene and interface layer. The junction is built from 2D primitives in scene.cpp. Rectangles make the road and pavement; lines and repeated shapes make lane markings, stop lines and crossings. Buildings, trees and benches give the scene context. The traffic lights and cars use shapes and colours that remain distinguishable against the road.")
speak("The map has a world coordinate system of 1000 by 800 units. A camera maps that world into the available viewport. When a vehicle is drawn, the scene uses the position and heading supplied by Elijah's model. The car shape is translated to that position and rotated to that angle. The signal drawing queries the model's current phase, so a red or green light on screen has an actual traffic state behind it.")
speak("The challenge was fitting a readable map into different window sizes. The camera preserves its proportions, and the renderer clips the world at the viewport edges. We increased the label size and changed the font for legibility. The expanded Fit view gives the junction more room during a presentation without changing the underlying world coordinates.")

heading("Kevin  Section 10  Controls and inspection")
operator("Scroll to 10 of 11. Point to the control panel, statistics, vehicle inspector and source card in sequence.")
speak("The interface is organised around how someone investigates the model. Start, Pause, Reset and Step are together. Demand, green duration and playback speed are nearby because they change the conditions of the experiment. Statistics sit above the scene, while the current signal phase is shown below it. This layout lets the viewer connect an action to the result without hunting around the window.")
speak("The route paths, labels and junction-boundary switches reveal layers when we need them, then hide them for a cleaner traffic view. Clicking a vehicle selects its live identifier and displays its approach, turn, position, rotation, speed and stopped time. The picking calculation maps the mouse position back into world coordinates, so selection still works when the viewport is scaled.")
speak("The UI calls the model methods rather than editing the drawing directly. For example, changing demand calls setDensity; Step calls step; Next phase calls requestNextPhase. Fit enlarges the simulation screen, and Back to controls returns to the dashboard. Our challenge was keeping those controls understandable while the state changed, so we grouped them by purpose and made the visible statistics update from the model.")

heading("Samuel  Section 11  Verification and collaboration")
operator("Scroll to 11 of 11. Show the GitHub screenshot as a snapshot. Do not describe its old commit count as the current state; the current repository will be shown live after the app demonstration.")
speak("My part is making the work reproducible and showing evidence beyond the screenshots. The build script first compiles and runs the calculation tests, then builds the application. The traffic tests cover all twelve continuous routes, seeded Reset, fixed-step independence, signal clearance and dense traffic safety. The graphics tests check Bresenham across octants, midpoint-circle results and clipping cases. During our build today the eight traffic checks and the graphics suite passed.")
speak("The README explains how to build and operate the app. The verification notes record what was checked. We coordinated work through GitHub branches and pull requests and used WhatsApp to discuss progress and resolve questions. The screenshot shows the repository structure; in the live browser we will show the current main branch and contribution history rather than rely on this older image.")
speak("The main lesson was that visible graphics need equally clear rules and checks behind them. For future development, pedestrian crossings would need defined spawn points, walking paths and safe timing with traffic signals. Other junctions or incidents would also need their own behaviour and tests. We are now going to demonstrate the actual program, then show the repository.")

heading("Live demonstration  One screen sharer")
operator("Click Simulation. Say what the controls and statistics show. Press Fit to enlarge the junction. Keep every camera visible.")
speak("This is the live junction rather than a screenshot. In Fit view the scene is enlarged, but the vehicle positions and signals still come from the same C++ model. The roads, crossings, traffic lights and labels are visible here. I will start the traffic so we can observe vehicles arriving and following their routes.")
operator("Press Start. Allow cars to appear. If the junction is quiet, return to controls, select Rush or increase demand, then press Fit again. Pause after a queue is visible. Select a car in the enlarged view.")
speak("Increasing demand changes how often new vehicles are requested. Vehicles wait at red and keep spacing behind other cars. The selected car's inspector shows its approach, turn, coordinates, heading, speed and waiting time. Those are values from the current Vehicle object. I will pause so you can see a stable state.")
operator("Click Back to controls. Press Step forward once. Point to the changed simulation time and vehicle positions. If useful, press Next phase and show the amber then all-red clearance before the next green.")
speak("Step advances one simulated second through fixed updates. Next phase does not skip the safety sequence. You can see the light and vehicle response together.")
operator("Open Graphics lab. Show Bresenham line and change an endpoint. Select Midpoint circle and adjust radius. Select Line clipping and change an endpoint so the accepted green part changes.")
speak("These three graphs are calculated by our C++ functions. The green line pixels, circle pixels and clipped segment change when the input changes. The gold drawing gives us a reference, which helps us explain exactly what the algorithm computed.")
operator("Switch to the browser with the current GitHub repository. Show the main branch, the relevant src and tests folders, documentation, and merged pull requests or commits that are actually present.")
speak("This repository contains the source we have just demonstrated. The model, graphics algorithms, scene and interface are separate files. The tests and documentation explain how to reproduce our results. The contribution history shows how the group assembled the work. Thank you.")

heading("Recording and submission check")
speak("Watch the recorded video once before submitting. Confirm all five cameras and the shared screen stay visible, the live traffic and graphics demonstrations can be read, and the GitHub URL shown is the repository being submitted. Upload one unlisted YouTube video and test its link in a private browser window. Submit that URL and the up-to-date GitHub URL in the course portal.")

doc.save(OUTPUT)
print(OUTPUT)
