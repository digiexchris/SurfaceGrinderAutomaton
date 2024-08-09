export type SiteConfig = typeof siteConfig;

export const siteConfig = {
  name: "Surface Grinder Automaton",
  description: "Let this thing be your hands.",
  navItems: [
    {
      label: "Home",
      href: "/",
    },
    {
      label: "Control",
      href: "/control",
    },
    {
      label: "Config",
      href: "/config",
    },
    {
      label: "Console",
      href: "/console",
    },
  ],
  navMenuItems: [
    {
      label: "Profile",
      href: "/profile",
    },
    {
      label: "Dashboard",
      href: "/dashboard",
    },
    {
      label: "Projects",
      href: "/projects",
    },
    {
      label: "Team",
      href: "/team",
    },
    {
      label: "Calendar",
      href: "/calendar",
    },
    {
      label: "Settings",
      href: "/settings",
    },
    {
      label: "Help & Feedback",
      href: "/help-feedback",
    },
    {
      label: "Logout",
      href: "/logout",
    },
  ],
  links: {
    github: "https://github.com/digiexchris/SurfaceGrinderAutomaton"
  },
};
