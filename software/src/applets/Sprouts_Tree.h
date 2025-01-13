#include "../HemisphereApplet.h"

class Sprouts : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Sprouts";
    }

	/* Run when the Applet is selected */
    void Start() {
        reseed();
    }

	/* Run during the interrupt service routine, 16667 times per second */
    void Controller() {
        
        /*
        // Example: Checking for clock signal and updating sequence
        if (Clock(0)) {
            // Update outputs
            Out(0, curr_pitch_cv);
            Out(1, curr_gate_cv);
        */
    }

	/* Draw the screen */
    void View() {
        branch_inc = 0;
        DrawTree(5, 34, 70, 18, 90);
    }

	/* Called when the encoder button for this hemisphere is pressed */
    void OnButtonPress() {
        reseed();
    }

	/* Called when the encoder for this hemisphere is rotated
	 * direction 1 is clockwise
	 * direction -1 is counterclockwise
	 */
    void OnEncoderMove(int direction) {
        selected_branch = constrain(selected_branch + direction, 0, 15);
        //reseed();
    }
        
    /* Each applet may save up to 32 bits of data. When data is requested from
     * the manager, OnDataRequest() packs it up (see HemisphereApplet::Pack()) and
     * returns it.
     */
    uint64_t OnDataRequest() {
        uint64_t data = 0;
        return data;
    }

    void OnDataReceive(uint64_t data) {
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "Digital in help";
        help[HEMISPHERE_HELP_CVS]      = "CV in help";
        help[HEMISPHERE_HELP_OUTS]     = "Out help";
        help[HEMISPHERE_HELP_ENCODER]  = "123456789012345678";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int branch_inc = 0;
    int selected_branch = 0;
    uint16_t seed; // The random seed that deterministically builds the sequence

    void DrawTree(int num_levels, int x0, int y0, double length, double angle) {
        srandom(seed);
        
        bool highlight = 0;

        if (num_levels == 0 || length < 2) return; // Stop condition

        // Calculate the end of the current branch
        int x1 = x0 + cos(angle * M_PI / 180.0) * length;
        int y1 = y0 - sin(angle * M_PI / 180.0) * length;

        // Add random offset to the end of the branch
        x1 += random(-length/10),(length/8); // Random shift in the x direction
        y1 += random(-length/10),(length/8); // Random shift in the y direction

        //Draw highlight if branch is selected
        if (num_levels == 1){
            
            if(selected_branch == branch_inc) {
                highlight = 1;
                gfxCircle(x1, y1, 3);
            }
            branch_inc++;
        }
        // Draw the branch
        gfxLine(x0, y0, x1, y1, highlight);

        // Recursively draw two branches
        DrawTree(num_levels - 1, x1, y1, length * 0.75, angle - 40); // Left branch
        DrawTree(num_levels - 1, x1, y1, length * 0.75, angle + 40); // Right branch
    }

    void reseed() {
        randomSeed(micros());
        seed = random(0, 65535); // 16 bits
    }

};

