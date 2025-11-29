// 1. Select the button and the body element
const toggleButton = document.getElementById('theme-toggle');
const body = document.body;

// 2. Add an event listener to detect when the button is clicked
toggleButton.addEventListener('click', () => {

    // 3. Toggle the 'dark-mode' class on the body
    body.classList.toggle('dark-mode');

    // 4. Update the button text based on the current mode
    if (body.classList.contains('dark-mode')) {
        toggleButton.textContent = "☀️ Switch to Light Mode";
    } else {
        toggleButton.textContent = "🌙 Switch to Dark Mode";
    }
});
